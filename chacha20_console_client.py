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

import socket, struct, os, sys
from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305
from cryptography.exceptions import InvalidTag

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

    try:
        return aead.decrypt(nonce, ct + tag, header)
    except InvalidTag:
        raise ValueError("Authentication tag mismatch")

def hchacha20(key: bytes, nonce16: bytes) -> bytes:
    """
    HChaCha20 subkey derivation — direct translation of hchacha20() in chacha_merged.c.
    Runs 20 ChaCha rounds on (key, nonce16) and returns words [0..3, 12..15]
    of the raw output (no initial-state addition), giving a 32-byte subkey.

    key:     32 bytes  (CHACHA_KEY master key)
    nonce16: 16 bytes  (session_prefix padded with 8 zero bytes)
    returns: 32 bytes  (per-session AEAD key)
    """
    SIGMA = b"expand 32-byte k"

    def rotl32(v: int, n: int) -> int:
        return ((v << n) | (v >> (32 - n))) & 0xFFFFFFFF

    def qr(s: list, a: int, b: int, c: int, d: int) -> None:
        s[a] = (s[a] + s[b]) & 0xFFFFFFFF;  s[d] = rotl32(s[d] ^ s[a], 16)
        s[c] = (s[c] + s[d]) & 0xFFFFFFFF;  s[b] = rotl32(s[b] ^ s[c], 12)
        s[a] = (s[a] + s[b]) & 0xFFFFFFFF;  s[d] = rotl32(s[d] ^ s[a],  8)
        s[c] = (s[c] + s[d]) & 0xFFFFFFFF;  s[b] = rotl32(s[b] ^ s[c],  7)

    # Initial ChaCha20 state: constants (0-3) | key (4-11) | nonce16 (12-15)
    state = list(struct.unpack_from('<16I', SIGMA + key + nonce16))

    for _ in range(10):          # 10 double-rounds = 20 rounds total
        qr(state, 0, 4,  8, 12) # column rounds
        qr(state, 1, 5,  9, 13)
        qr(state, 2, 6, 10, 14)
        qr(state, 3, 7, 11, 15)
        qr(state, 0, 5, 10, 15) # diagonal rounds
        qr(state, 1, 6, 11, 12)
        qr(state, 2, 7,  8, 13)
        qr(state, 3, 4,  9, 14)

    # Output: first 4 words + last 4 words (words 12-15) — NOT added back to
    # the initial state. This is what distinguishes HChaCha20 from ChaCha20.
    return struct.pack('<8I',
        state[0],  state[1],  state[2],  state[3],
        state[12], state[13], state[14], state[15])


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
    # We send random contribution to session nonce -> AIBO then XOR into sessionNonce before first AEAD frame
    # Bytes [8..11] are counter-controlled on both sides
    client_nonce = os.urandom(NONCE_SIZE)
    s.sendall(client_nonce)
    session_prefix = bytes(a ^ b for a, b in zip(raw_nonce[:8], client_nonce[:8]))

    # Derive a per-session AEAD key: HChaCha20(master_key, session_prefix || 0x00*8)
    # Mirrors the derivation in TinyConsole.cc ReceiveCont phase 2.
    hchacha_input = session_prefix + b'\x00' * 8   # 8-byte prefix padded to 16 bytes
    session_key   = hchacha20(CHACHA_KEY, hchacha_input)

    tx_counter = 0
    rx_counter = 0
    aead        = ChaCha20Poly1305(session_key)

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
            except (ValueError, InvalidTag):
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