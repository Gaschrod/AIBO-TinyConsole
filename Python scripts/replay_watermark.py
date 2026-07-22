"""
replay_watermark.py
Client-side rollback / replay detection for TinyConsole.

The robot embeds a persistent, monotonically increasing session counter in
bytes [0..3] of the 12-byte handshake nonce (little-endian). 
As that value is part of the transcript both parties sign, it is authenticated: once the
robot's signature verifies, the counter is known to have been chosen by the
genuine robot for this session.

This module persists, per robot identity, the highest counter value accepted so
far and rejects any session whose counter does not strictly increase. That
catches a rolled-back or cloned robot (e.g. a Memory Stick restored from an old
backup) and any attempt to replay a recorded robot handshake to the client.

Storage: a small JSON file keyed by the robot's Ed25519 public key (hex), so the
watermark follows the robot's identity rather than its IP address.
"""

import json
import os
import tempfile
from typing import Optional

_DEFAULT_PATH = os.path.join(
    os.path.expanduser("~"), ".config", "aibo", "replay_watermarks.json"
)


def extract_counter(raw_nonce: bytes) -> int:
    """Return the robot's session counter from the 12-byte handshake nonce."""
    if len(raw_nonce) < 4:
        raise ValueError("nonce too short to contain a session counter")
    return int.from_bytes(raw_nonce[0:4], "little")


def load(path: str) -> dict:
    try:
        with open(path, "r") as f:
            return json.load(f)
    except (FileNotFoundError, ValueError):
        return {}


def atomic_save(path: str, data: dict) -> None:
    directory = os.path.dirname(path)
    os.makedirs(directory, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=directory)
    try:
        with os.fdopen(fd, "w") as f:
            json.dump(data, f)
            f.flush()
            os.fsync(f.fileno())
        os.replace(tmp, path) 
    finally:
        if os.path.exists(tmp):
            try:
                os.remove(tmp)
            except OSError:
                pass


def check_and_update(robot_pubkey_hex: str,
                     server_counter: int,
                     path: Optional[str] = None) -> None:
    """
    Enforce strict monotonicity of the robot's session counter.

    Call only after the robot's handshake signature has verified -> counter authenticated.
    Raises ConnectionError if the counter does not strictly exceed the highest value previously accepted for this robot. 
    Onsuccess, persists the new higher value.
    """
    store_path = path or _DEFAULT_PATH
    store = load(store_path)
    key = robot_pubkey_hex.lower()
    last = store.get(key, -1)

    if server_counter <= last:
        raise ConnectionError(
            "Replay/rollback suspected: the robot's session counter "
            f"({server_counter}) did not increase past the last accepted value "
            f"({last}). The robot's anti-replay counter may have been rolled "
            "back (e.g. a Memory Stick restored from backup), or an old "
            "handshake is being replayed. Refusing to proceed. If you "
            "deliberately reset the robot's counter, delete its entry in "
            f"{store_path}."
        )

    store[key] = server_counter
    atomic_save(store_path, store)
