"""
aibo_link.py
Low-level encrypted TCP link to the AIBO ERS-7 TinyConsole.

Extracted from chacha20_console_client.py.  The wire protocol, crypto
primitives, and nonce arithmetic are unchanged — only the structure changes
from a standalone script to a reusable class.

Protocol recap (half-duplex, client always sends first):
  1. TCP connect
  2. Server sends banner line + 12-byte session nonce (plaintext)
  3. Client sends 12-byte random nonce contribution (plaintext)
     → bytes [0..7] of both are XOR'd to form the session prefix
  4. A per-session AEAD key is derived via:
       HChaCha20(master_key, session_prefix || 0x00*8)
     This mirrors the derivation in TinyConsole.cc ReceiveCont phase 2.
     The master key is never used directly for encryption.
  5. Every subsequent message is an RFC-7539-style AEAD frame:
       [ uint16 LE ciphertext_length ][ ciphertext ][ 16-byte Poly1305 tag ]
     Nonce = session_prefix[0..7] + msg_counter[8..11]
     Client-TX counter has high bit clear; server-TX has high bit set.
"""

import os, socket, struct
from typing import Optional

from cryptography.exceptions import InvalidTag
from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305

NONCE_SIZE     = 12
FRAME_TAG_SIZE = 16


# ------------------------------------------------------------------ helpers --

def _recv_exact(sock: socket.socket, n: int) -> bytes:
    """Blocking read of exactly n bytes."""
    buf = b""
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("Connection closed by server")
        buf += chunk
    return buf


def _build_nonce(session_prefix: bytes, counter: int, is_server_tx: bool) -> bytes:
    """
    Build the 12-byte nonce for one AEAD message.
      session_prefix  8-byte prefix (XOR of server + client contributions)
      counter         per-direction uint32 counter, starts at 0 each session
      is_server_tx    True for server→client direction (sets bit 31 of counter)
    """
    mc = counter
    if is_server_tx:
        mc |= 0x80000000   # Set high bit for server-to-client messages

    return session_prefix + struct.pack("<I", mc)


def _aead_encrypt(aead: ChaCha20Poly1305, nonce: bytes, plaintext: bytes) -> bytes:
    """Return a complete wire frame: [2-byte LE length][ciphertext][16-byte tag]."""
    header     = struct.pack("<H", len(plaintext))
    ct_and_tag = aead.encrypt(nonce, plaintext, header)  # Header = additional authenticated data (AAD)
    ct         = ct_and_tag[:-FRAME_TAG_SIZE]
    tag        = ct_and_tag[-FRAME_TAG_SIZE:]
    return header + ct + tag


def _aead_decrypt(aead: ChaCha20Poly1305, nonce: bytes, sock: socket.socket) -> bytes:
    """
    Read one AEAD frame from sock, authenticate and decrypt it.
    Raises ValueError on tag mismatch, ConnectionError on EOF.
    """
    header = _recv_exact(sock, 2)
    ct_len = struct.unpack("<H", header)[0]

    body   = _recv_exact(sock, ct_len + FRAME_TAG_SIZE)
    ct     = body[:ct_len]
    tag    = body[ct_len:]

    try:
        return aead.decrypt(nonce, ct + tag, header)
    except InvalidTag:
        raise ValueError("Authentication tag mismatch")

def _hchacha20(key: bytes, nonce16: bytes) -> bytes:
    """
    HChaCha20 subkey derivation — direct Python translation of hchacha20()
    in chacha_merged.c.

    Runs 20 ChaCha20 rounds on (key, nonce16) and returns words [0..3] and
    [12..15] of the raw output state (WITHOUT adding back the initial state).
    That omission is exactly what distinguishes HChaCha20 from ChaCha20 and
    what makes it safe to use as a key-derivation function.

    key:     32 bytes  — master CHACHA_KEY
    nonce16: 16 bytes  — session_prefix (8 bytes) padded with 8 zero bytes
    returns: 32 bytes  — per-session AEAD key
    """
    SIGMA = b"expand 32-byte k"

    def rotl32(v: int, n: int) -> int:
        return ((v << n) | (v >> (32 - n))) & 0xFFFFFFFF

    def qr(s: list, a: int, b: int, c: int, d: int) -> None:
        s[a] = (s[a] + s[b]) & 0xFFFFFFFF; s[d] = rotl32(s[d] ^ s[a], 16)
        s[c] = (s[c] + s[d]) & 0xFFFFFFFF; s[b] = rotl32(s[b] ^ s[c], 12)
        s[a] = (s[a] + s[b]) & 0xFFFFFFFF; s[d] = rotl32(s[d] ^ s[a],  8)
        s[c] = (s[c] + s[d]) & 0xFFFFFFFF; s[b] = rotl32(s[b] ^ s[c],  7)

    # Initial ChaCha20 state: constants[0..3] | key[4..11] | nonce16[12..15]
    state = list(struct.unpack_from("<16I", SIGMA + key + nonce16))

    for _ in range(10):           # 10 double-rounds = 20 rounds total
        qr(state,  0,  4,  8, 12) # column rounds
        qr(state,  1,  5,  9, 13)
        qr(state,  2,  6, 10, 14)
        qr(state,  3,  7, 11, 15)
        qr(state,  0,  5, 10, 15) # diagonal rounds
        qr(state,  1,  6, 11, 12)
        qr(state,  2,  7,  8, 13)
        qr(state,  3,  4,  9, 14)

    # Output words [0..3] + [12..15] — initial state NOT added back.
    return struct.pack("<8I",
        state[0],  state[1],  state[2],  state[3],
        state[12], state[13], state[14], state[15],
    )


# ---------------------------------------------------------------- AiboLink --

class AiboLink:
    """
    Manages a single encrypted TCP session to TinyConsole.

    Typical usage::

        link = AiboLink("192.168.1.124", 7777, key_bytes)
        banner = link.connect()
        response = link.send_command("GET_UP")
        link.disconnect()          # sends QUIT, then closes
        # or on error:
        link.close()               # force-closes without QUIT
    """

    def __init__(self, robot_ip: str, port: int, key: bytes) -> None:
        if len(key) != 32:
            raise ValueError(f"ChaCha20 key must be 32 bytes, got {len(key)}")
        self._robot_ip = robot_ip
        self._port     = port
        self._key      = key

        self._sock:           Optional[socket.socket]  = None
        self._aead:           Optional[ChaCha20Poly1305] = None
        self._session_prefix: Optional[bytes]          = None
        self._tx_counter = 0
        self._rx_counter = 0

    # ----------------------------------------------------------------

    @property
    def is_connected(self) -> bool:
        return self._sock is not None

    # ----------------------------------------------------------------

    def connect(self) -> str:
        """
        Open a TCP connection to the AIBO and complete the nonce handshake.

        Returns the banner string (e.g. "CONSOLE_READY").
        Raises ConnectionError / OSError on network failure.
        """
        if self._sock:
            raise RuntimeError("Already connected; call close() first")

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5.0) # 5s to connect
        s.connect((self._robot_ip, self._port))
        s.settimeout(10.0)  # per-command timeout once connected ; adjust depending on needs

        # Read banner line (terminated by \n)
        banner = b""
        while b"\n" not in banner:
            banner += _recv_exact(s, 1)

        # Read 12-byte server nonce (plaintext)
        raw_nonce = _recv_exact(s, NONCE_SIZE)

        # Send 12 random bytes as our nonce contribution
        client_nonce = os.urandom(NONCE_SIZE)
        s.sendall(client_nonce)

        # XOR bytes [0..7] to build the session prefix.
        # Bytes [8..11] are the per-message counter, managed by _build_nonce.
        self._session_prefix = bytes(
            a ^ b for a, b in zip(raw_nonce[:8], client_nonce[:8])
        )

        # Derive a per-session AEAD key so that the master key is never used
        # directly for encryption.  Input nonce is the 8-byte session prefix
        # zero-padded to the 16 bytes that HChaCha20 expects.
        # This mirrors TinyConsole.cc ReceiveCont phase 2.
        hchacha_input = self._session_prefix + b"\x00" * 8
        session_key   = _hchacha20(self._key, hchacha_input)

        self._sock       = s
        self._aead       = ChaCha20Poly1305(session_key)
        self._tx_counter = 0
        self._rx_counter = 0

        return banner.decode("ascii", errors="replace").strip()

    # ----------------------------------------------------------------

    def send_command(self, cmd: str) -> str:
        """
        Encrypt and send one command string, then receive and decrypt the
        AIBO's response.

        The trailing newline required by TinyConsole is added automatically.
        Returns the AIBO's plaintext response (stripped of whitespace).

        Raises:
          ConnectionError  if not connected or if the socket closes mid-transfer
          ValueError       if the Poly1305 tag doesn't match
        """
        if not self.is_connected:
            raise ConnectionError("Not connected to AIBO")

        plaintext = (cmd.rstrip("\r\n") + "\n").encode("utf-8")

        # --- Encrypt and send ---
        tx_nonce = _build_nonce(self._session_prefix, self._tx_counter,
                                 is_server_tx=False)
        self._tx_counter += 1
        frame = _aead_encrypt(self._aead, tx_nonce, plaintext)
        self._sock.sendall(frame)

        # --- Receive and decrypt ---
        rx_nonce = _build_nonce(self._session_prefix, self._rx_counter,
                                 is_server_tx=True)
        self._rx_counter += 1
        response = _aead_decrypt(self._aead, rx_nonce, self._sock)
        return response.decode("utf-8", errors="replace").strip()

    # ----------------------------------------------------------------

    def close(self) -> None:
        """
        Force-close the TCP socket immediately without sending QUIT.
        Safe to call even if already disconnected.
        Use this after a link error to reset state before reconnecting.
        """
        if self._sock:
            try:
                self._sock.close()
            except OSError:
                pass
            finally:
                self._sock           = None
                self._aead           = None
                self._session_prefix = None
                self._tx_counter     = 0
                self._rx_counter     = 0

    def disconnect(self) -> None:
        """
        Graceful shutdown: send QUIT (best-effort), then close the socket.
        Silently ignores any errors during the QUIT exchange.
        """
        if self._sock:
            try:
                self.send_command("QUIT")
            except Exception:
                pass
            self.close()
