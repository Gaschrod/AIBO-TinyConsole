#!/usr/bin/env python3
"""
Inline MITM test for the AIBO TinyConsole ChaCha20-Poly1305 / Ed25519 protocol.

Does *NOT* sniff-and-reinject at L2/L3 -> transparent TCP proxy that uses the same TinyConsole wire format, 
logs every field, and can actively attack the stream. 

Five functions:

    --mode passthrough   Baseline which produces the capture logs used by nonce_audit.py /
                         cipher_sample.py / replay_session.py 
                         If robot can't communicate with client when this is used,
                         then need to check network/firewall/NAT/routing/encryption

    --mode replay        Duplicate the client's first command frame to the robot. 
                         The robot's rx counter has already advanced, 
                         so the duplicate decrypts under the wrong nonce
                         -> Poly1305 fails -> robot does NOT execute twice
                         If this does not happen, then there is an error to verify

    --mode tamper        Flip one bit in the first client -> robot frame
                         (ciphertext or the 16-byte tag). 
                         AEAD verify must fail -> robot refuses the frame. 
                         Used to prove integrity

    --mode mitm-key      Corrupt/replace the robot's handshake signature
                         (i.e. try to substitute a key at the handshake). 
                         The client checks against his copy of the robot pubkey 
                         -> InvalidSignature -> client aborts. 
                         Used to prove that the signature scheme works.

    --mode rst           After N frames, send a TCP RST to both ends to
                         force a connection drop.
                         FIN is not used as it's a graceful termination 
                         and we want to test the effect of a forceful termination.
                         Uses SO_LINGER=0 so a genuine RST is emitted.

Format of frames (must match ConsoleConfig.h / chacha20_console_client.py):
    Handshake, robot -> client:   banner line "...\n" | raw_nonce[12] | robot_sig[64]
        where raw_nonce[12] is the os.urandom[12] contribution to the robot's nonce

    Handshake, client -> robot:   client_nonce[12] | client_sig[64]

    Then, both directions, repeated AEAD frames:
        header[2] = uint16 LE = 256   (PAD_BLOCK, constant on the wire)
        ciphertext[256]
        tag[16]
    -> each post-handshake frame is exactly 274 bytes (can change depending on the value of the parameter controlling the fixed size)

How to run:
    Explicit (simplest, no kernel config): point the *client* at this proxy:
        
        # On the hotspot used to spoof the access point (Linux)
        python3 aibo_mitm.py --listen 0.0.0.0:7777 --robot 192.168.1.124:7777 --mode tamper
        
        # On the client, run the script against the supposed legitimate access point (will also works with ROS 2 nodes):
        python3 chacha20_console_client.py --ip <HOTSPOT_IP> --port 7777

    Transparent (client unmodified): redirect the robot port to the proxy with
    nftables, and start the proxy with --transparent so it recovers the original
    destination via SO_ORIGINAL_DST:
        nft add table ip mitm
        nft add chain ip mitm prerouting '{ type nat hook prerouting priority -100; }'
        nft add rule ip mitm prerouting ip daddr 192.168.1.124 tcp dport 7777 redirect to :7777
        python3 aibo_mitm.py --listen 0.0.0.0:7777 --transparent --mode passthrough
"""

import argparse
import os
import socket
import struct
import sys
import threading
import json
from datetime import datetime, timezone

# ---- Protocol constants (must match the client/OPEN-R configuration) -------------------
NONCE_SIZE = 12
SIG_SIZE = 64
TAG_SIZE = 16
PAD_BLOCK = 256
HEADER_SIZE = 2
FRAME_SIZE = HEADER_SIZE + PAD_BLOCK + TAG_SIZE   # 274

SO_ORIGINAL_DST = 80   # Linux-only constant


def now_iso():
    return datetime.now(timezone.utc).isoformat()


def recv_exact(sock, n):
    """Read exactly n bytes or raise ConnectionError on EOF"""
    buf = b""
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("peer closed connection")
        buf += chunk
    return buf


def recv_line(sock, limit=512):
    """Read one banner line up to and including b'\\n'"""
    buf = b""
    while b"\n" not in buf:
        buf += recv_exact(sock, 1)
        if len(buf) > limit:
            raise ValueError("banner too long -> wire format mismatch?")
    return buf


class SessionLog:
    """One JSONL file per proxied connection plus raw per-direction byte dumps
    Structured input for the audit/sampling/replay scripts"""

    def __init__(self, outdir):
        os.makedirs(outdir, exist_ok=True)
        stamp = datetime.now().strftime("%Y%m%d-%H%M%S-%f")
        self.path = os.path.join(outdir, f"session-{stamp}.jsonl")
        self.raw_c2r_path = os.path.join(outdir, f"session-{stamp}.c2r.bin")
        self.raw_r2c_path = os.path.join(outdir, f"session-{stamp}.r2c.bin")
        self._lock = threading.Lock()
        self._c2r = open(self.raw_c2r_path, "wb")
        self._r2c = open(self.raw_r2c_path, "wb")
        self.frames_saved = 0

    def event(self, **kw):
        kw["ts"] = now_iso()
        with self._lock:
            with open(self.path, "a") as f:
                f.write(json.dumps(kw) + "\n")

    def raw(self, direction, data):
        with self._lock:
            (self._c2r if direction == "c2r" else self._r2c).write(data)

    def close(self):
        with self._lock:
            self._c2r.close()
            self._r2c.close()


def emit_rst(sock):
    """Force a TCP RST by zeroing the linger timeout"""
    try:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER,
                        struct.pack("ii", 1, 0))
        sock.close()
    except OSError:
        pass


class Proxy:
    def __init__(self, args, client_sock, robot_sock, log):
        self.args = args
        self.c = client_sock       # socket to the real client
        self.r = robot_sock        # socket to the real robot
        self.log = log
        self.frames_seen = 0
        self._stop = threading.Event()
        self._counter_lock = threading.Lock()

    def forge_robot_sig(self, sig):
        if self.args.mode != "mitm-key":
            return sig, False
        # No need to modify more than 1 byte to check if rejection of invalid 
        # signature is well implemented (equivalent to using a different key to generate anoter signature)
        forged = bytearray(sig)
        forged[0] ^= 0x01
        self.log.event(event="ATTACK", mode="mitm-key",
                       detail="robot handshake signature corrupted; client must "
                              "reject against pinned ROBOT_PUBKEY_HEX")
        return bytes(forged), True

    def tamper_frame(self, frame, direction):
        if self.args.mode != "tamper" or direction != "c2r":
            return frame, False
        if self.frames_seen != 0:      # only hit the first command frame
            return frame, False
        target = self.args.tamper_field   # "ct" or "tag"
        b = bytearray(frame)
        if target == "tag":
            idx = HEADER_SIZE + PAD_BLOCK + (self.args.tamper_index % TAG_SIZE)
        else:
            idx = HEADER_SIZE + (self.args.tamper_index % PAD_BLOCK)
        bit = 1 << (self.args.tamper_bit % 8)
        b[idx] ^= bit
        self.log.event(event="ATTACK", mode="tamper",
                       detail=f"flipped bit {self.args.tamper_bit} of byte {idx} "
                              f"(field={target}); AEAD verify must fail")
        return bytes(b), True

    def replay_frame(self, frame, direction):
        """Return an extra copy to inject (in-session duplicate), or None."""
        if self.args.mode != "replay" or direction != "c2r":
            return None
        if self.frames_seen != 0:
            return None
        self.log.event(event="ATTACK", mode="replay",
                       detail="duplicating first client->robot frame in-session; "
                              "robot rx counter advanced -> nonce mismatch -> reject")
        return frame

    # --- handshake relaying -------------------------------------------------
    def relay_handshake(self):
        # robot -> client: banner
        banner = recv_line(self.r)
        self.log.raw("r2c", banner)
        self.log.event(event="banner", bytes=len(banner),
                       text=banner.decode("ascii", "replace").strip())
        self.c.sendall(banner)

        # robot -> client: raw_nonce[12]
        raw_nonce = recv_exact(self.r, NONCE_SIZE)
        self.log.raw("r2c", raw_nonce)
        self.log.event(event="robot_nonce", hex=raw_nonce.hex(),
                       counter_le32=int.from_bytes(raw_nonce[0:4], "little"))
        self.c.sendall(raw_nonce)

        # client -> robot: client_nonce[12]
        client_nonce = recv_exact(self.c, NONCE_SIZE)
        self.log.raw("c2r", client_nonce)
        self.log.event(event="client_nonce", hex=client_nonce.hex())
        self.r.sendall(client_nonce)

        # robot -> client: robot_sig[64]  (attack point for mitm-key)
        robot_sig = recv_exact(self.r, SIG_SIZE)
        self.log.raw("r2c", robot_sig)
        forwarded_sig, forged = self.forge_robot_sig(robot_sig)
        self.log.event(event="robot_sig", hex=robot_sig.hex(), forged=forged)
        self.c.sendall(forwarded_sig)

        # client -> robot: client_sig[64]
        client_sig = recv_exact(self.c, SIG_SIZE)
        self.log.raw("c2r", client_sig)
        self.log.event(event="client_sig", hex=client_sig.hex())
        self.r.sendall(client_sig)

        # Record the derived session prefix for the nonce audit.
        prefix = bytes(a ^ b for a, b in zip(raw_nonce[:8], client_nonce[:8]))
        self.log.event(event="session_prefix", hex=prefix.hex())

    # --- one direction of the encrypted frame phase ------------------------
    def pump_frames(self, src, dst, direction):
        try:
            while not self._stop.is_set():
                header = recv_exact(src, HEADER_SIZE)
                length = struct.unpack("<H", header)[0]
                if length != PAD_BLOCK:
                    self.log.event(event="WARN",
                                   detail=f"unexpected block size {length}")
                body = recv_exact(src, length + TAG_SIZE)
                frame = header + body

                self.log.raw(direction, frame)
                with self._counter_lock:
                    idx = self.frames_seen
                self.log.event(event="frame", dir=direction, index=idx,
                               ct_hex=body[:length].hex(),
                               tag_hex=body[length:].hex())

                out, _tampered = self.tamper_frame(frame, direction)
                dup = self.replay_frame(frame, direction)

                dst.sendall(out)
                if dup is not None:
                    dst.sendall(dup)   # malicious duplicate

                with self._counter_lock:
                    self.frames_seen += 1
                    self.log.frames_saved = self.frames_seen
                    total = self.frames_seen

                if self.args.mode == "rst" and total >= self.args.rst_after:
                    self.log.event(event="ATTACK", mode="rst",
                                   detail=f"emitting RST to both ends after "
                                          f"{total} frame(s)")
                    self._stop.set()
                    emit_rst(self.c)
                    emit_rst(self.r)
                    return
        except (ConnectionError, OSError) as e:
            self.log.event(event="closed", dir=direction, reason=str(e))
            self._stop.set()

    def run(self):
        try:
            self.relay_handshake()
        except Exception as e:
            self.log.event(event="handshake_error", reason=str(e))
            emit_rst(self.c)
            emit_rst(self.r)
            return
        t1 = threading.Thread(target=self.pump_frames,
                              args=(self.c, self.r, "c2r"), daemon=True)
        t2 = threading.Thread(target=self.pump_frames,
                              args=(self.r, self.c, "r2c"), daemon=True)
        t1.start(); t2.start()
        t1.join(); t2.join()
        for s in (self.c, self.r):
            try:
                s.close()
            except OSError:
                pass


def original_dst(sock):
    """Recover the pre-DNAT destination for transparent (nftables redirect) mode."""
    data = sock.getsockopt(socket.SOL_IP, SO_ORIGINAL_DST, 16)
    port = struct.unpack("!H", data[2:4])[0]
    ip = socket.inet_ntoa(data[4:8])
    return ip, port


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--listen", default="0.0.0.0:7777",
                    help="host:port to listen on (default 0.0.0.0:7777)")
    ap.add_argument("--robot", default=None,
                    help="host:port of the real robot (required unless --transparent)")
    ap.add_argument("--transparent", action="store_true",
                    help="recover destination via SO_ORIGINAL_DST (needs nftables redirect)")
    ap.add_argument("--mode", default="passthrough",
                    choices=["passthrough", "replay", "tamper", "mitm-key", "rst"])
    ap.add_argument("--outdir", default="./captures",
                    help="directory for per-session logs (default ./captures)")

    ap.add_argument("--tamper-field", default="ct", choices=["ct", "tag"])
    ap.add_argument("--tamper-index", type=int, default=0)
    ap.add_argument("--tamper-bit", type=int, default=0)

    ap.add_argument("--rst-after", type=int, default=1,
                    help="RST after this many frames (mode=rst)")
    ap.add_argument("--once", action="store_true",
                    help="handle a single connection then exit")
    args = ap.parse_args()

    if not args.transparent and not args.robot:
        ap.error("--robot host:port is required unless --transparent is set")

    lhost, lport = args.listen.rsplit(":", 1)
    lsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    lsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    if args.transparent:
        try:
            lsock.setsockopt(socket.SOL_IP, 19, 1)   # IP_TRANSPARENT
        except OSError as e:
            print(f"[!] IP_TRANSPARENT failed ({e}); run as root.", file=sys.stderr)
    lsock.bind((lhost, int(lport)))
    lsock.listen(8)
    os.makedirs(args.outdir, exist_ok=True)
    print(f"[*] aibo_mitm listening on {args.listen}  mode={args.mode}")
    print(f"[*] captures -> {os.path.abspath(args.outdir)}")
    if not args.transparent:
        print(f"[*] forwarding to robot at {args.robot}")
    print("[*] waiting for a connection... "
          "(if nothing arrives, traffic is NOT being redirected here -- "
          "check `nft list ruleset` counters and routing, not bridging)")

    try:
        while True:
            client_sock, peer = lsock.accept()
            print(f"[+] client connected from {peer}")
            try:
                if args.transparent:
                    rip, rport = original_dst(client_sock)
                else:
                    rip, rport = args.robot.rsplit(":", 1)
                    rport = int(rport)
                robot_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                robot_sock.connect((rip, rport))
                print(f"[+] connected to robot {rip}:{rport}")
            except OSError as e:
                print(f"[!] cannot reach robot: {e}")
                emit_rst(client_sock)
                if args.once:
                    break
                continue

            log = SessionLog(args.outdir)
            print(f"[*] logging to {log.path}")
            Proxy(args, client_sock, robot_sock, log).run()
            log.close()
            print(f"[-] session finished ({log.frames_saved} frames relayed)")
            if args.once:
                break
    except KeyboardInterrupt:
        print("\n[*] interrupted; shutting down cleanly.")
    finally:
        lsock.close()


main()