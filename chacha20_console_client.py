#!/usr/bin/env python3
#
# chacha_console_client.py
# Console client for TinyConsole (ChaCha20+Poly1305 AEAD version).
#
# Protocol recap (must match TinyConsole.cc exactly):
#
#   1. TCP connect
#   2. Server sends "CONSOLE_READY\n" (14 bytes, plaintext)
#   3. Server sends 12-byte session nonce  (plaintext)
#      nonce[0..3]  = session ID (LE)
#      nonce[4..7]  = client IPv4 (LE)
#      nonce[8..11] = 0x00000000  (placeholder, ignored by client)
#
#   From here every message is an AEAD frame:
#      [ uint16_t ciphertext_length  (2 bytes, LE) ]
#      [ ciphertext                  (ctLen bytes) ]
#      [ Poly1305 tag                (16 bytes)    ]
#
#   Nonce construction per message:
#      nonce[0..7]  = session_prefix received from server
#      nonce[8..11] = msg_counter (LE), shared across TX and RX,
#                     incremented after every message sent or received.
#
#   Counter sequence (half-duplex: client always sends first):
#      0 → client TX (server decrypts with msgCounter_=0)
#      1 → server TX (client decrypts with msgCounter_=1)
#      2 → client TX (server decrypts with msgCounter_=2)
#      ...
#
# Dependency:
#   pip install cryptography
#

import socket
import struct
import sys
from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305

# ---------------------------------------------------------------
#  Configuration  — must match ConsoleConfig.h exactly
# ---------------------------------------------------------------
ROBOT_IP  = "192.168.1.124"
PORT      = 7777
NONCE_SIZE     = 12
FRAME_TAG_SIZE = 16

CHACHA_KEY = bytes([
    0x40, 0x77, 0x81, 0xED, 0x6A, 0x6D, 0x04, 0xC2,
    0x7F, 0x24, 0xFB, 0x68, 0x87, 0x16, 0x22, 0x1E,
    0x8B, 0x8B, 0xE3, 0x1E, 0x6D, 0x32, 0xC8, 0x6A,
    0x1F, 0xC6, 0x2F, 0x38, 0xC6, 0xD0, 0x69, 0x4C
])

# ---------------------------------------------------------------
#  Helpers
# ---------------------------------------------------------------

def recv_exact(sock: socket.socket, n: int) -> bytes:
    """Read exactly n bytes from sock, blocking until available."""
    buf = b""
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("Connection closed by server")
        buf += chunk
    return buf


def build_nonce(session_prefix: bytes, counter: int, is_server_tx: bool) -> bytes:
    """
    Reconstruct the 12-byte nonce for one message.
      session_prefix : bytes [0..7] received from server
      counter        : uint32 shared message counter (LE in bytes [8..11])
      is_server_tx   : bool indicating if this is a server-to-client message
    """
    mc = counter

    if is_server_tx:
        mc |= 0x80000000   # Set high bit for server-to-client messages
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
    Raises ValueError on authentication failure, ConnectionError on EOF.
    """
    header = recv_exact(sock, 2)
    ct_len = struct.unpack("<H", header)[0]

    body = recv_exact(sock, ct_len + FRAME_TAG_SIZE)
    ct  = body[:ct_len]
    tag = body[ct_len:]

    return aead.decrypt(nonce, ct + tag, header) 


# ---------------------------------------------------------------
#  Main
# ---------------------------------------------------------------

def main():
    # -- Connect --
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((ROBOT_IP, PORT))
        print(f"Connected to robot at {ROBOT_IP}:{PORT}")
    except Exception as e:
        print(f"Connection error: {e}")
        sys.exit(1)

    # -- Handshake: read banner line then 12-byte nonce --
    banner = b""
    while b"\n" not in banner:
        banner += recv_exact(s, 1)
    print(f"Banner: {banner.decode('ascii', errors='replace').strip()}")

    raw_nonce      = recv_exact(s, NONCE_SIZE)
    session_prefix = raw_nonce[:8]   # bytes [0..7]: session ID + client IP
    # bytes [8..11] are 0x00 placeholders from the server; we manage our
    # own counter from 0 in lock-step with the server's msgCounter_.

    tx_counter = 0
    rx_counter = 0
    aead        = ChaCha20Poly1305(CHACHA_KEY)

    # -- Command loop --
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
            tx_nonce    = build_nonce(session_prefix, tx_counter, is_server_tx=False)
            tx_counter += 1
            frame       = aead_encrypt(aead, tx_nonce, plaintext)
            s.sendall(frame)

            # 2. Receive and decrypt
            try:
                rx_nonce    = build_nonce(session_prefix, rx_counter, is_server_tx=True)
                rx_counter += 1
                response    = aead_decrypt(aead, rx_nonce, s)
                print("ROBOT:", response.decode("utf-8", errors="replace").strip())
            except ValueError:
                print("ERROR: authentication tag mismatch — dropping response")
            except ConnectionError:
                print("Server closed the connection.")
                break

            if cmd.strip().upper() == "QUIT":
                break

    except KeyboardInterrupt:
        print("\nDisconnecting...")
    finally:
        s.close()


if __name__ == "__main__":
    main()