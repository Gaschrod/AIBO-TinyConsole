#!/usr/bin/env python3
#
# chacha20_console_client.py
# Self-contained console client for TinyConsole (ChaCha20+Poly1305 AEAD,
# Ed25519-authenticated handshake).
#
# Recap (must match TinyConsole.cc!):
#
#   1. TCP connect
#   2. Robot sends "CONSOLE_READY\n" (14 bytes, plaintext)
#   3. Robot sends 12-byte session nonce  (plaintext)
#      nonce[0..3]  = session ID (LE)
#      nonce[4..7]  = client IPv4 (LE)
#      nonce[8..11] = 0x00000000  (placeholder, ignored by client)
#   4. Client sends its own 12-byte random nonce contribution (plaintext)
#   5. Robot sends a 64-byte Ed25519 signature (plaintext) over
#      (HANDSHAKE_CONTEXT || its raw nonce || client's raw nonce),
#      signed with its persistent identity key. The client verifies this
#      against ROBOT_PUBKEY_HEX before trusting anything further. 
#
#   From here every message is an AEAD frame:
#      [ uint16_t ciphertext_length  (2 bytes, LE) ]
#      [ ciphertext                  (ctLen bytes) ]
#      [ Poly1305 tag                (16 bytes)    ]
#
#   Nonce construction per message:
#      nonce[0..7]  = session_prefix (XOR of robot + client raw nonces)
#      nonce[8..11] = msg_counter (LE), shared across TX and RX,
#                     incremented after every message sent or received.
#
#   Counter sequence (half-duplex: client always sends first):
#      0 → client TX (Robot decrypts with msgCounter_=0)
#      1 → Robot TX (client decrypts with msgCounter_=1)
#      2 → client TX (Robot decrypts with msgCounter_=2)
#      ...
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
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
from cryptography.exceptions import InvalidTag, InvalidSignature

# ---------------------------------------------------------------
#  Configuration  — must match ConsoleConfig.h exactly
#
#  Robot IP/port are parameters
# ---------------------------------------------------------------
DEFAULT_ROBOT_IP = "192.168.1.124"
DEFAULT_PORT     = 7777

NONCE_SIZE         = 12
FRAME_TAG_SIZE     = 16
HANDSHAKE_SIG_SIZE = 64
# Must match HANDSHAKE_CONTEXT in ConsoleConfig.h byte-for-byte.
HANDSHAKE_CONTEXT  = b"AIBO-TinyConsole-Handshake"

CHACHA_KEY = bytes([
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
])

# PLACEHOLDER, need to be filled with the public key of the robot.
# Left empty, this raises an error.
ROBOT_PUBKEY_HEX = ""

# ---------------------------------------------------------------
#  Helpers
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
    Encrypt plaintext and return a complete wire frame:
      [2-byte LE ciphertext length][ciphertext][16-byte Poly1305 tag]
    """
    header = struct.pack("<H", len(plaintext))

    ct_and_tag = aead.encrypt(nonce, plaintext, header)   # Header = additional authenticated data (AAD)
    ct  = ct_and_tag[:-FRAME_TAG_SIZE]
    tag = ct_and_tag[-FRAME_TAG_SIZE:]
    return header + ct + tag


def aead_decrypt(aead: ChaCha20Poly1305,
                 nonce: bytes,
                 sock: socket.socket) -> bytes:
    """
    Read one AEAD frame from sock, verify its tag, and return the plaintext.
    Raises ValueError on authentication failure.
    """
    header = recv_exact(sock, 2)
    ct_len = struct.unpack("<H", header)[0]

    body = recv_exact(sock, ct_len + FRAME_TAG_SIZE)
    ct  = body[:ct_len]
    tag = body[ct_len:]

    try:
        return aead.decrypt(nonce, ct + tag, header)
    except InvalidTag:
        raise ValueError("Authentication tag mismatch")


# -----------
#  Handshake 
# -----------

def do_handshake(s: socket.socket,
                 robot_pubkey_obj: Ed25519PublicKey) -> Tuple[str, bytes, bytes]:
    """
    Read the banner, exchange nonces, verify the robot's handshake
    signature, and derive the per-session AEAD key.

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
            "ROBOT_PUBKEY_HEX is not set. Run robot_keys_generator.py once "
            "and paste its printed robot_ed25519_pubkey_hex into this file."
        )
        sys.exit(1)
    try:
        robot_pubkey_obj = Ed25519PublicKey.from_public_bytes(bytes.fromhex(ROBOT_PUBKEY_HEX))
    except ValueError as e:
        print(f"ROBOT_PUBKEY_HEX is not valid: {e}")
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
        banner, session_prefix, session_key = do_handshake(s, robot_pubkey_obj)
    except (ConnectionError, OSError) as e:
        print(f"Handshake failed: {e}")
        s.close()
        sys.exit(1)

    print(f"Banner: {banner}")
    print("Robot identity verified against ROBOT_PUBKEY_HEX.")

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
