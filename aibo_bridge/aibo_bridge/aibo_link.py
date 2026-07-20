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
  4. Server signs (HANDSHAKE_CONTEXT || its raw nonce || client's raw
     nonce) with its persistent Ed25519 identity key and sends the
     64-byte detached signature (plaintext). The client verifies this
     against `robot_pubkey` -- pinned out of band via identity_keygen.py,
     NOT learned from the wire -- before trusting anything further.
     This is what lets a client tell "the real robot" apart from
     anything else that merely has a copy of `key` (the ChaCha key),
     which is far more exposed than the Ed25519 secret ever needs to be.
  5. Mutual authentication: the client signs the identical transcript
     the robot just signed (HANDSHAKE_CONTEXT || robot's raw nonce ||
     client's raw nonce) with its own persistent Ed25519 key and sends
     the 64-byte detached signature (plaintext). The robot verifies it
     against the publiv kry CLIENT_ED25519_PK in ConsoleConfig.h before
     accepting any encrypted traffic. No extra round trip: this rides
     ahead of the first AEAD command frame. Matches TinyConsole.cc's
     RX_CLIENT_SIG phase byte-for-byte.
  6. Every subsequent message is an RFC-7539-style AEAD frame:
       [ uint16 LE ciphertext_length ][ ciphertext ][ 16-byte Poly1305 tag ]
     Nonce = session_prefix[0..7] + msg_counter[8..11]
     Client-TX counter has high bit clear; server-TX has high bit set.
"""

import os, socket, struct
from typing import Optional

from cryptography.exceptions import InvalidTag, InvalidSignature
from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305
from cryptography.hazmat.primitives.asymmetric.ed25519 import (
    Ed25519PublicKey, Ed25519PrivateKey,
)

NONCE_SIZE     = 12
HANDSHAKE_SIG_SIZE = 64
# Must match HANDSHAKE_CONTEXT in ConsoleConfig.h byte-for-byte.
HANDSHAKE_CONTEXT  = b"AIBO-TinyConsole-Handshake"
FRAME_TAG_SIZE = 16


# ------------------------------------------------------------------

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

# ------------------------------------------------------------------

class AiboLink:
    """
    Manages a single encrypted TCP session to TinyConsole.

    Typical usage::

        link = AiboLink("192.168.1.124", 7777, key_bytes,
                        robot_pubkey_bytes, client_seed_bytes)
        banner = link.connect()
        response = link.send_command("GET_UP")
        link.disconnect()          # sends QUIT, then closes
        # or on error:
        link.close()               # force-closes without QUIT
    """

    def __init__(self, robot_ip: str, port: int, key: bytes,
                 robot_pubkey: bytes, client_seed: bytes) -> None:
        if len(key) != 32:
            raise ValueError(f"ChaCha20 key must be 32 bytes, got {len(key)}")
        if len(robot_pubkey) != 32:
            raise ValueError(f"robot_pubkey must be 32 bytes, got {len(robot_pubkey)}")
        if len(client_seed) != 32:
            raise ValueError(f"client_seed must be 32 bytes, got {len(client_seed)}")
        self.robot_ip = robot_ip
        self.port     = port
        self.key      = key

        # Pinned out of band (identity_keygen.py's printed hex), NOT learned
        # from the wire -- deliberately stricter than literal trust-on-first-
        # use, which would accept whatever key shows up on the very first
        # connection with no external check at all.
        self.robot_pubkey_obj = Ed25519PublicKey.from_public_bytes(robot_pubkey)

        # The client's own persistent Ed25519 identity. 
        self.client_privkey_obj = Ed25519PrivateKey.from_private_bytes(client_seed)

        self.sock:           Optional[socket.socket]  = None
        self.aead:           Optional[ChaCha20Poly1305] = None
        self.session_prefix: Optional[bytes]          = None
        self.tx_counter = 0
        self.rx_counter = 0

    # ----------------------------------------------------------------

    @property
    def is_connected(self) -> bool:
        return self.sock is not None

    # ----------------------------------------------------------------

    def connect(self) -> str:
        """
        Open a TCP connection to the AIBO and complete the nonce handshake.

        Returns the banner string (e.g. "CONSOLE_READY").
        Raises ConnectionError / OSError on network failure.
        """
        if self.sock:
            raise RuntimeError("Already connected; call close() first")

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5.0) # 5s to connect
        s.connect((self.robot_ip, self.port))
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

        # --- Verify the robot's identity before trusting anything else ---
        # The signature covers exactly (context || raw_nonce || client_nonce),
        # matching SignHandshake() in TinyConsole.cc byte-for-byte.
        sig = _recv_exact(s, HANDSHAKE_SIG_SIZE)
        transcript = HANDSHAKE_CONTEXT + raw_nonce + client_nonce
        try:
            self.robot_pubkey_obj.verify(sig, transcript)
        except InvalidSignature:
            s.close()
            raise ConnectionError(
                "Robot identity verification FAILED for "
                f"{self.robot_ip}:{self.port}. The device that answered "
                "did not sign the handshake with the expected key. This "
                "could mean an impersonator/MITM on the network, or that "
                "robot_pubkey is stale after the robot was re-flashed with "
                "a new identity. Refusing to proceed."
            )

        # --- Prove OUR identity to the robot (mutual auth) ---
        # Sign the identical transcript the robot just signed. The robot holds
        # this same message (the two nonces) and verifies our signature against
        # its saved CLIENT_ED25519_PK (TinyConsole.cc RX_CLIENT_SIG phase).
        client_sig = self.client_privkey_obj.sign(transcript)
        s.sendall(client_sig)

        # XOR bytes [0..7] to build the session prefix.
        # Bytes [8..11] are the per-message counter, managed by _build_nonce.
        self.session_prefix = bytes(
            a ^ b for a, b in zip(raw_nonce[:8], client_nonce[:8])
        )

        self.sock       = s
        self.aead       = ChaCha20Poly1305(self.key)
        self.tx_counter = 0
        self.rx_counter = 0

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
        tx_nonce = _build_nonce(self.session_prefix, self.tx_counter,
                                 is_server_tx=False)
        self.tx_counter += 1
        frame = _aead_encrypt(self.aead, tx_nonce, plaintext)
        self.sock.sendall(frame)

        # --- Receive and decrypt ---
        rx_nonce = _build_nonce(self.session_prefix, self.rx_counter,
                                 is_server_tx=True)
        self.rx_counter += 1
        response = _aead_decrypt(self.aead, rx_nonce, self.sock)
        return response.decode("utf-8", errors="replace").strip()

    # ----------------------------------------------------------------

    def close(self) -> None:
        """
        Force-close the TCP socket immediately without sending QUIT.
        Safe to call even if already disconnected.
        Use this after a link error to reset state before reconnecting.
        """
        if self.sock:
            try:
                self.sock.close()
            except OSError:
                pass
            finally:
                self.sock           = None
                self.aead           = None
                self.session_prefix = None
                self.tx_counter     = 0
                self.rx_counter     = 0

    def disconnect(self) -> None:
        """
        Graceful shutdown: send QUIT (best-effort), then close the socket.
        Silently ignores any errors during the QUIT exchange.
        """
        if self.sock:
            try:
                self.send_command("QUIT")
            except Exception:
                pass
            self.close()
