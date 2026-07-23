#!/usr/bin/env python3
#
# chacha20_console_client.py
# Self-contained console client for TinyConsole (ChaCha20+Poly1305 AEAD,
# Ed25519-authenticated handshake).
#
# Usage:
#   Interactive:
#     python3 chacha20_console_client.py --ip 192.168.1.124
#   Latency benchmark (RTT of one command, repeated):
#     python3 chacha20_console_client.py --ip 192.168.1.124 \
#             --benchmark --iterations 1000 --warmup 20 --command PING \
#             --csv aead_rtt.csv
#   Client-side crypto micro-benchmark (encrypt+decrypt cost, no network):
#     python3 chacha20_console_client.py --crypto-microbench --iterations 5000
#
# Measurement:
#   - RTT is measured with time.perf_counter_ns() around encrypt -> send ->
#     recv -> decrypt (full client-observed round trip) 
#     Use PING (robot answers PONG) -> no motor "side effect" (non-measured latency)
#   - Subtract the --crypto-microbench figure from the RTT to attribute the
#     residual to network + robot-side compute.
#
# Dependency:
#   pip install cryptography

import argparse, socket, struct, os, sys, time, statistics, csv
from typing import Tuple, List

from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305
from cryptography.hazmat.primitives.asymmetric.ed25519 import (
    Ed25519PublicKey, Ed25519PrivateKey,
)
from cryptography.exceptions import InvalidTag, InvalidSignature
from replay_watermark import check_and_update, extract_counter

# ---------------------------------------------------------------
#  Configuration  — must match ConsoleConfig.h exactly
#
#  Robot IP/port are parameters
# ---------------------------------------------------------------
DEFAULT_ROBOT_IP = "192.168.1.124"
DEFAULT_PORT = 7777

NONCE_SIZE = 12
FRAME_TAG_SIZE = 16
HANDSHAKE_SIG_SIZE = 64

PAD_BLOCK = 256        # fixed padded-plaintext size (must match ConsoleConfig.h)
PAD_LEN_PREFIX = 2     # inner uint16 LE real length

# Must match HANDSHAKE_CONTEXT in ConsoleConfig.h byte-for-byte.
HANDSHAKE_CONTEXT  = b"AIBO-TinyConsole-Handshake"

CHACHA_KEY = bytes([
])

# PLACEHOLDER, need to be filled with the public key of the robot.
# Left empty, this raises an error.
ROBOT_PUBKEY_HEX = ""

# The client's private Ed25519 key (32-byte hex). !SECRET!
# The corresponding public key must be written in ConsoleConfig.h (robot side)
# and is used to verify the client's identity during the
# handshake. Left empty, this raises an error (fail closed).
CLIENT_PRIVATE_KEY_HEX = ""

# ---------------------------------------------------------------
#  Functions
# ---------------------------------------------------------------

def recv_exact(sock: socket.socket, n: int) -> bytes:
    """Read exactly n bytes from sock, blocking until available."""
    buf = b""
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("Connection closed by robot")
        buf += chunk
    return buf


def build_nonce(session_prefix: bytes, counter: int, is_robot_tx: bool) -> bytes:
    """
    Reconstruct the 12-byte nonce for one message.
      session_prefix : bytes [0..7] from the (verified) handshake
      counter : uint32 shared message counter (LE in bytes [8..11])
      is_robot_tx : bool indicating if this is a robot-to-client message
    """
    mc = counter
    if is_robot_tx:
        mc |= 0x80000000   # Set high bit for robot-to-client messages
    return session_prefix + struct.pack("<I", mc)


def aead_encrypt(aead: ChaCha20Poly1305,
                 nonce: bytes,
                 plaintext: bytes) -> bytes:
    """
    Pad plaintext to a fixed block and return a complete wire frame:
      [2-byte LE block size (constant)][ciphertext: PAD_BLOCK][16-byte tag]
    Inner padded layout: [uint16 LE real_len][payload][zero padding].
    """
    if len(plaintext) > PAD_BLOCK - PAD_LEN_PREFIX:
        raise ValueError(
            f"message too large for one block: {len(plaintext)} "
            f"(max {PAD_BLOCK - PAD_LEN_PREFIX})"
        )
    padded = (struct.pack("<H", len(plaintext)) + plaintext).ljust(
        PAD_BLOCK, b"\x00")
    header = struct.pack("<H", PAD_BLOCK)           # constant on the wire
    ct_and_tag = aead.encrypt(nonce, padded, header)  # header = AAD
    ct  = ct_and_tag[:-FRAME_TAG_SIZE]
    tag = ct_and_tag[-FRAME_TAG_SIZE:]
    return header + ct + tag


def aead_decrypt(aead: ChaCha20Poly1305,
                 nonce: bytes,
                 sock: socket.socket) -> bytes:
    """
    Read one fixed-size AEAD frame, verify its tag, remove padding and return the
    *real* plaintext. Raises ValueError on authentication failure.
    """
    header = recv_exact(sock, 2)
    ct_len = struct.unpack("<H", header)[0]
    if ct_len != PAD_BLOCK:
        raise ValueError(f"unexpected block size {ct_len} (expected {PAD_BLOCK})")

    body = recv_exact(sock, ct_len + FRAME_TAG_SIZE)
    ct  = body[:ct_len]
    tag = body[ct_len:]

    try:
        padded = aead.decrypt(nonce, ct + tag, header)
    except InvalidTag:
        raise ValueError("Authentication tag mismatch")

    real_len = struct.unpack_from("<H", padded, 0)[0]
    if real_len > PAD_BLOCK - PAD_LEN_PREFIX:
        raise ValueError(f"bad inner length {real_len}")
    return padded[PAD_LEN_PREFIX:PAD_LEN_PREFIX + real_len]


# ---------------------------------------------------------------
#  One timed request/response exchange
# ---------------------------------------------------------------

def exchange(s: socket.socket, aead: ChaCha20Poly1305, session_prefix: bytes,
             tx_counter: int, rx_counter: int,
             plaintext: bytes) -> Tuple[bytes, int, int, int]:
    """
    Send one encrypted command and read exactly one response frame.
    Returns (response_plaintext, new_tx_counter, new_rx_counter, rtt_ns).

    The timed window covers encrypt -> send -> recv -> decrypt: the full
    client-observed round trip. Counters are threaded through explicitly so
    the AEAD nonce sequence stays identical to normal operation.
    """
    t0 = time.perf_counter_ns()
    tx_nonce = build_nonce(session_prefix, tx_counter, is_robot_tx=False)
    frame    = aead_encrypt(aead, tx_nonce, plaintext)
    s.sendall(frame)

    rx_nonce = build_nonce(session_prefix, rx_counter, is_robot_tx=True)
    response = aead_decrypt(aead, rx_nonce, s)
    t1 = time.perf_counter_ns()
    return response, tx_counter + 1, rx_counter + 1, t1 - t0


# ---------------------------------------------------------------
#  Measurement helpers (shared column format with the XOR client)
# ---------------------------------------------------------------

def _percentile(sorted_ns: List[int], p: float) -> float:
    n = len(sorted_ns)
    if n == 1:
        return float(sorted_ns[0])
    k = (n - 1) * p
    f = int(k)
    c = min(f + 1, n - 1)
    return sorted_ns[f] + (sorted_ns[c] - sorted_ns[f]) * (k - f)


def summarize(name: str, samples_ns: List[int]) -> None:
    """Print a min/mean/median/p95/p99/max/stdev table in milliseconds."""
    if not samples_ns:
        print(f"\n=== {name}: no samples ===")
        return
    xs = sorted(samples_ns)
    n = len(xs)
    ms = lambda v: v / 1e6
    mean = statistics.fmean(xs)
    stdev = statistics.pstdev(xs) if n > 1 else 0.0
    print(f"\n=== {name}  (n={n}) ===")
    print(f"  min    {ms(xs[0]):9.3f} ms")
    print(f"  mean   {ms(mean):9.3f} ms")
    print(f"  median {ms(_percentile(xs, 0.50)):9.3f} ms")
    print(f"  p95    {ms(_percentile(xs, 0.95)):9.3f} ms")
    print(f"  p99    {ms(_percentile(xs, 0.99)):9.3f} ms")
    print(f"  max    {ms(xs[-1]):9.3f} ms")
    print(f"  stdev  {ms(stdev):9.3f} ms")


def write_csv(path: str, samples_ns: List[int]) -> None:
    """Dump raw per-sample RTTs (microseconds) for plotting in the thesis."""
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["sample", "rtt_us"])
        for i, v in enumerate(samples_ns):
            w.writerow([i, f"{v / 1e3:.3f}"])
    print(f"  wrote {len(samples_ns)} samples to {path}")


def run_benchmark(s: socket.socket, aead: ChaCha20Poly1305,
                  session_prefix: bytes, args: argparse.Namespace) -> None:
    plaintext = (args.command + "\n").encode("utf-8")
    tx = rx = 0

    # Warm-up (discarded): first exchanges pay ARP resolution, socket ramp, etc.
    for _ in range(args.warmup):
        _, tx, rx, _ = exchange(s, aead, session_prefix, tx, rx, plaintext)
        if args.delay:
            time.sleep(args.delay)

    samples: List[int] = []
    for _ in range(args.iterations):
        resp, tx, rx, dt = exchange(s, aead, session_prefix, tx, rx, plaintext)
        samples.append(dt)
        if args.delay:
            time.sleep(args.delay)

    summarize(f"RTT '{args.command}' (AEAD ChaCha20-Poly1305)", samples)
    if args.csv:
        write_csv(args.csv, samples)

    # Best-effort clean close.
    try:
        exchange(s, aead, session_prefix, tx, rx, b"QUIT\n")
    except Exception:
        pass


def crypto_microbench(session_prefix: bytes, iterations: int) -> None:
    """
    Client-side ChaCha20-Poly1305 cost per command: one full-block encrypt +
    one decrypt, no network. Must be substracted from the RTT to attribute
    the remainder to the network and the robot.
    """
    aead = ChaCha20Poly1305(CHACHA_KEY)
    payload = os.urandom(PAD_BLOCK)
    header = struct.pack("<H", PAD_BLOCK)
    samples: List[int] = []
    for i in range(iterations):
        nonce = build_nonce(session_prefix, i & 0x7FFFFFFF, is_robot_tx=False)
        t0 = time.perf_counter_ns()
        ct = aead.encrypt(nonce, payload, header)
        aead.decrypt(nonce, ct, header)
        t1 = time.perf_counter_ns()
        samples.append(t1 - t0)
    summarize("Client ChaCha20-Poly1305 encrypt+decrypt (local, per command)",
              samples)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Encrypted, identity-verified console client for the AIBO TinyConsole."
    )
    parser.add_argument("--ip", "-i", default=DEFAULT_ROBOT_IP,
                        help=f"AIBO IP address (default: {DEFAULT_ROBOT_IP})")
    parser.add_argument("--port", "-p", type=int, default=DEFAULT_PORT,
                        help=f"TinyConsole TCP port (default: {DEFAULT_PORT})")
    # --- measurement options ---
    parser.add_argument("--benchmark", action="store_true",
                        help="Measure round-trip latency instead of the interactive REPL.")
    parser.add_argument("--iterations", type=int, default=1000,
                        help="Number of measured samples (default: 1000).")
    parser.add_argument("--warmup", type=int, default=20,
                        help="Warm-up exchanges discarded before measuring (default: 20).")
    parser.add_argument("--command", default="PING",
                        help="Command to time; use a side-effect-free one like PING (default: PING).")
    parser.add_argument("--delay", type=float, default=0.0,
                        help="Optional seconds to sleep between exchanges (default: 0).")
    parser.add_argument("--csv", default=None,
                        help="Write raw per-sample RTTs (microseconds) to this CSV file.")
    parser.add_argument("--crypto-microbench", action="store_true",
                        help="Measure local ChaCha20-Poly1305 encrypt+decrypt cost (no network) and exit.")
    return parser.parse_args()


def main():
    args = parse_args()

    # Local crypto micro-benchmark needs no robot / no keys.
    if args.crypto_microbench:
        # A fixed dummy prefix is fine: we are timing the primitive, not a session.
        crypto_microbench(session_prefix=b"\x00" * 8, iterations=args.iterations)
        return

    if not ROBOT_PUBKEY_HEX:
        print(
            "ROBOT_PUBKEY_HEX is not set. Run keys_generator.py with robot argument once "
            "and paste its printed robot_ed25519_pubkey_hex into this file."
        )
        sys.exit(1)
    try:
        robot_pubkey_obj = Ed25519PublicKey.from_public_bytes(bytes.fromhex(ROBOT_PUBKEY_HEX))
    except ValueError as e:
        print(f"ROBOT_PUBKEY_HEX is not valid: {e}")
        sys.exit(1)

    if not CLIENT_PRIVATE_KEY_HEX:
        print(
            "CLIENT_PRIVATE_KEY_HEX is not set. Run keys_generator.py with client argument once "
            "and paste its printed client_ed25519_seed_hex into this file."
        )
        sys.exit(1)
    try:
        client_privkey_obj = Ed25519PrivateKey.from_private_bytes(
            bytes.fromhex(CLIENT_PRIVATE_KEY_HEX))
    except ValueError as e:
        print(f"CLIENT_PRIVATE_KEY_HEX is not valid: {e}")
        sys.exit(1)

    # Connect (timed)
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        t0 = time.perf_counter_ns()
        s.connect((args.ip, args.port))
        t_connect = time.perf_counter_ns() - t0
        print(f"Connected to robot at {args.ip}:{args.port}")
    except Exception as e:
        print(f"Connection error: {e}")
        sys.exit(1)

    # Handshake (timed): banner, nonce exchange, mutual signature verification
    try:
        t0 = time.perf_counter_ns()
        banner, session_prefix, session_key = do_handshake(
            s, robot_pubkey_obj, client_privkey_obj)
        t_handshake = time.perf_counter_ns() - t0
    except (ConnectionError, OSError) as e:
        print(f"Handshake failed: {e}")
        s.close()
        sys.exit(1)

    print(f"Banner: {banner}")
    print("Robot identity verified against ROBOT_PUBKEY_HEX.")
    print("Client identity signature sent (robot verifies against pinned key).")
    print(f"[timing] TCP connect: {t_connect/1e6:.3f} ms | "
          f"handshake (nonce + mutual Ed25519): {t_handshake/1e6:.3f} ms")

    aead = ChaCha20Poly1305(session_key)

    # Benchmark mode: measure and exit.
    if args.benchmark:
        try:
            run_benchmark(s, aead, session_prefix, args)
        finally:
            s.close()
        return

    tx_counter = 0
    rx_counter = 0

    # Command loop (interactive) — now also prints per-command RTT.
    try:
        while True:
            try:
                cmd = input("AIBO> ")
            except EOFError:
                break
            if not cmd:
                continue

            plaintext = (cmd + "\n").encode("utf-8")
            try:
                response, tx_counter, rx_counter, dt = exchange(
                    s, aead, session_prefix, tx_counter, rx_counter, plaintext)
                print("ROBOT:", response.decode("utf-8", errors="replace").strip(),
                      f"  [rtt {dt/1e6:.3f} ms]")
            except (ValueError, InvalidTag):
                print("ERROR: authentication tag mismatch — dropping response")
            except ConnectionError:
                print("Robot closed the connection.")
                break

            if cmd.strip().upper() == "QUIT":
                break

    except KeyboardInterrupt:
        print("\nDisconnecting...")
    finally:
        s.close()


# ---------------------------------------------------------------
#  Handshake
# ---------------------------------------------------------------

def do_handshake(s: socket.socket,
                 robot_pubkey_obj: Ed25519PublicKey,
                 client_privkey_obj: Ed25519PrivateKey) -> Tuple[str, bytes, bytes]:
    """
    Read the banner, exchange nonces, verify the robot's handshake
    signature, prove our identity to the robot, and derive the
    per-session AEAD key.

    Mutual authentication: after verifying the robot, the client signs the
    same transcript (HANDSHAKE_CONTEXT || robot_nonce || client_nonce) with
    its own private key and sends the 64-byte detached signature. The robot
    checks it against the saved CLIENT_ED25519_PK before accepting any
    encrypted traffic.

    Returns (banner_text, session_prefix, session_key).
    Raises ConnectionError if identity verification fails, or if the
    connection drops mid-handshake.
    """
    banner = b""
    while b"\n" not in banner:
        banner += recv_exact(s, 1)

    raw_nonce = recv_exact(s, NONCE_SIZE)

    # Send a random contribution to the session nonce, the robot XORs
    # it into its own. Bytes [8..11] are counter-controlled on both sides -> only [0..7] matter
    client_nonce = os.urandom(NONCE_SIZE)
    s.sendall(client_nonce)

    # --- Verify the robot's identity before trusting anything else ---
    # Transcript must match SignHandshake() in TinyConsole.cc byte-for-byte:
    # HANDSHAKE_CONTEXT || raw_nonce (as the robot sent it) || client_nonce.
    sig = recv_exact(s, HANDSHAKE_SIG_SIZE)
    transcript = HANDSHAKE_CONTEXT + raw_nonce + client_nonce
    try:
        robot_pubkey_obj.verify(sig, transcript)
    except InvalidSignature:
        raise ConnectionError(
            "Robot identity verification FAILED. The device that answered "
            "did not sign the handshake with the expected key. This could "
            "mean an impersonator/MITM on the network, or that "
            "ROBOT_PUBKEY_HEX is stale after the robot was re-flashed with "
            "a new identity. Refusing to proceed."
        )

    # Now ensures that the robot's session counter value is strictly increasing, to prevent replay/rollback attacks.
    check_and_update(ROBOT_PUBKEY_HEX, extract_counter(raw_nonce))

    # --- Prove OUR identity to the robot (mutual auth) ---
    # Sign the identical transcript the robot just signed. The robot holds
    # this same message (the two nonces) and verifies the client's signature
    # against its saved CLIENT_ED25519_PK. No extra round trip: this rides
    # ahead of the first AEAD command frame.
    client_sig = client_privkey_obj.sign(transcript)
    s.sendall(client_sig)

    session_prefix = bytes(a ^ b for a, b in zip(raw_nonce[:8], client_nonce[:8]))

    return banner.decode("ascii", errors="replace").strip(), session_prefix, CHACHA_KEY


main()
