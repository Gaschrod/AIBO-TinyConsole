#!/usr/bin/env python3
#
# chacha20_console_client.py
# Self-contained console client for TinyConsole (ChaCha20+Poly1305 AEAD,
# Ed25519-authenticated handshake).
#
# Usage:
#   python3 chacha20_console_client.py
#   python3 chacha20_console_client.py --ip 192.168.1.42 --port 7777
#
# Dependency:
#   pip install cryptography

import argparse, socket, struct, os, sys
from typing import Tuple

from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305
from cryptography.hazmat.primitives.asymmetric.ed25519 import (
    Ed25519PublicKey, Ed25519PrivateKey,
)
from cryptography.exceptions import InvalidTag, InvalidSignature

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
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
])

# PLACEHOLDER, need to be filled with the public key of the robot.
# Left empty, this raises an error.
ROBOT_PUBKEY_HEX = ""

# The client's private Ed25519 key (32-byte hex). !SECRET!
# The corresponding public key must be written in ConsoleConfig.h (robot side) 
# and is used to verify the client's identity during the
# handshake. Left empty, this raises an error (fail closed).
CLIENT_SEED_HEX = ""

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

    # --- Prove OUR identity to the robot (mutual auth) ---
    # Sign the identical transcript the robot just signed. The robot holds
    # this same message (the two nonces) and verifies the client's signature
    # against its saved CLIENT_ED25519_PK. No extra round trip: this rides
    # ahead of the first AEAD command frame.
    client_sig = client_privkey_obj.sign(transcript)
    s.sendall(client_sig)

    session_prefix = bytes(a ^ b for a, b in zip(raw_nonce[:8], client_nonce[:8]))

    return banner.decode("ascii", errors="replace").strip(), session_prefix, CHACHA_KEY


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Encrypted, identity-verified console client for the AIBO TinyConsole."
    )
    parser.add_argument(
        "--ip", "-i", default=DEFAULT_ROBOT_IP,
        help=f"AIBO IP address (default: {DEFAULT_ROBOT_IP})",
    )
    parser.add_argument(
        "--port", "-p", type=int, default=DEFAULT_PORT,
        help=f"TinyConsole TCP port (default: {DEFAULT_PORT})",
    )
    return parser.parse_args()


def main():
    args = parse_args()

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

    if not CLIENT_SEED_HEX:
        print(
            "CLIENT_SEED_HEX is not set. Run keys_generator.py with client argument once "
            "and paste its printed client_ed25519_seed_hex into this file."
        )
        sys.exit(1)
    try:
        client_privkey_obj = Ed25519PrivateKey.from_private_bytes(
            bytes.fromhex(CLIENT_SEED_HEX))
    except ValueError as e:
        print(f"CLIENT_SEED_HEX is not valid: {e}")
        sys.exit(1)

    # Connect
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((args.ip, args.port))
        print(f"Connected to robot at {args.ip}:{args.port}")
    except Exception as e:
        print(f"Connection error: {e}")
        sys.exit(1)

    # Handshake: banner, nonce exchange, signature verification
    try:
        banner, session_prefix, session_key = do_handshake(
            s, robot_pubkey_obj, client_privkey_obj)
    except (ConnectionError, OSError) as e:
        print(f"Handshake failed: {e}")
        s.close()
        sys.exit(1)

    print(f"Banner: {banner}")
    print("Robot identity verified against ROBOT_PUBKEY_HEX.")
    print("Client identity signature sent (robot verifies against pinned key).")

    tx_counter = 0
    rx_counter = 0
    aead = ChaCha20Poly1305(session_key)

    # Command loop
    try:
        while True:
            try:
                cmd = input("AIBO> ")
            except EOFError:
                break
            if not cmd:
                continue

            plaintext = (cmd + "\n").encode("utf-8")

            # 1. Encrypt and send
            tx_nonce    = build_nonce(session_prefix, tx_counter, is_robot_tx=False)
            tx_counter += 1
            frame       = aead_encrypt(aead, tx_nonce, plaintext)
            s.sendall(frame)

            # 2. Receive and decrypt
            try:
                rx_nonce    = build_nonce(session_prefix, rx_counter, is_robot_tx=True)
                rx_counter += 1
                response    = aead_decrypt(aead, rx_nonce, s)
                print("ROBOT:", response.decode("utf-8", errors="replace").strip())
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

main()