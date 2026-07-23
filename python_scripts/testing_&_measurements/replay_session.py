#!/usr/bin/env python3
"""
Takes the client -> robot bytes captured from an earlier session (the *.c2r.bin
file written by aibo_mitm.py in passthrough mode) and replays them into
a fresh TCP connection to the robot.

Why it must fail: the robot issues a NEW handshake nonce for every connection.
The recorded client signature signed the OLD transcript
    HANDSHAKE_CONTEXT || old_robot_nonce || old_client_nonce
but the robot now verifies against
    HANDSHAKE_CONTEXT || NEW_robot_nonce || old_client_nonce
so Ed25519 verification of the replayed client_sig fails and the robot drops the
connection before any command frame is accepted. Even if the client signature was
skipped, the AEAD session prefix derives from the new nonce so the replayed
command frames decrypt under the wrong key/nonce and Poly1305 rejects them.

Expected result: robot closes the connection (or never executes the command).

Usage:
    python3 replay_session.py --robot <IP:PORT> --capture captures/session-XXus.c2r.bin
"""

import argparse
import socket
import sys

NONCE_SIZE = 12
SIG_SIZE = 64


def recv_exact(sock, n):
    buf = b""
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("robot closed connection")
        buf += chunk
    return buf


def recv_line(sock, limit=512):
    buf = b""
    while b"\n" not in buf:
        buf += recv_exact(sock, 1)
        if len(buf) > limit:
            raise ValueError("banner too long")
    return buf


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--robot", required=True, help="host:port of the robot")
    ap.add_argument("--capture", required=True,
                    help="a *.c2r.bin file from a passthrough capture")
    ap.add_argument("--timeout", type=float, default=5.0)
    args = ap.parse_args()

    with open(args.capture, "rb") as f:
        c2r = f.read()

    # Recorded client->robot stream layout: client_nonce[12] | client_sig[64] | frames...
    if len(c2r) < NONCE_SIZE + SIG_SIZE:
        print("[!] capture too short to contain a handshake", file=sys.stderr)
        sys.exit(2)
    rec_client_nonce = c2r[:NONCE_SIZE]
    rec_client_sig = c2r[NONCE_SIZE:NONCE_SIZE + SIG_SIZE]
    rec_frames = c2r[NONCE_SIZE + SIG_SIZE:]

    host, port = args.robot.rsplit(":", 1)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(args.timeout)
    s.connect((host, int(port)))
    print(f"[*] fresh connection to robot {args.robot}")

    try:
        banner = recv_line(s)
        print(f"[*] robot banner: {banner.decode('ascii','replace').strip()}")
        new_robot_nonce = recv_exact(s, NONCE_SIZE)
        print(f"[*] robot issued a NEW nonce: {new_robot_nonce.hex()}")

        # Replay the OLD client nonce + OLD client signature.
        s.sendall(rec_client_nonce)
        print(f"[*] replayed old client_nonce: {rec_client_nonce.hex()}")
        new_robot_sig = recv_exact(s, SIG_SIZE)
        s.sendall(rec_client_sig)
        print(f"[*] replayed old client_sig:   {rec_client_sig.hex()[:32]}...")

        # Try to push the old command frames too.
        if rec_frames:
            s.sendall(rec_frames)
            print(f"[*] replayed {len(rec_frames)} bytes of old command frames")

        # If anti-replay holds, the robot rejects and closes -> recv returns EOF.
        try:
            resp = s.recv(512)
            if not resp:
                print("\n[PASS] Robot dropped the replayed session (EOF). "
                      "Cross-session replay rejected.")
            else:
                print(f"\n[FAIL] Robot answered {len(resp)} bytes to a replayed "
                      f"session -- investigate: {resp[:64]!r}")
        except (socket.timeout, ConnectionError):
            print("\n[PASS] No valid response / connection reset. "
                  "Cross-session replay rejected.")
    except (ConnectionError, ValueError) as e:
        print(f"\n[PASS] Robot refused the replay during handshake ({e}).")
    finally:
        s.close()

main()