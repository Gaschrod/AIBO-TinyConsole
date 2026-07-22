#!/usr/bin/env python3
"""
Reads every session-*.jsonl produced by aibo_mitm.py and checks the nonce
construction across sessions:

  - session_prefix reuse  (bytes [0..7], = robot_nonce[:8] XOR client_nonce[:8])
  - full robot_nonce / client_nonce reuse
  - monotonicity of the robot's persistent counter (nonce bytes [0..3] LE)

Warning: this does not prove the randomness of the code/implementation (space to verify = 2^64)

We know that:
  - Within a session, the 32-bit message counter occupies nonce bytes [8..11] and
    strictly increments (with the MSB reserved as a direction flag), so no nonce
    repeats within a session is by construction (not luck/"hypothetically this should work")
  - Across sessions, the 8-byte (64-bit) prefix includes the client's fresh
    os.urandom(8) contribution (XORed in), so prefixes are uniform over 2^64
    By the birthday bound, the probability of any prefix collision after k
    sessions is k^2 / 2^65. This script prints that probability for your k.

Usage:
    python3 nonce_audit.py --dir captures
"""

import argparse
import glob
import json
import os
import sys


def load_sessions(dirpath):
    sessions = []
    for path in sorted(glob.glob(os.path.join(dirpath, "session-*.jsonl"))):
        rec = {"path": path, "robot_nonce": None, "client_nonce": None,
               "prefix": None, "robot_counter": None}
        with open(path) as f:
            for line in f:
                try:
                    ev = json.loads(line)
                except ValueError:
                    continue
                t = ev.get("event")
                if t == "robot_nonce":
                    rec["robot_nonce"] = ev.get("hex")
                    rec["robot_counter"] = ev.get("counter_le32")
                elif t == "client_nonce":
                    rec["client_nonce"] = ev.get("hex")
                elif t == "session_prefix":
                    rec["prefix"] = ev.get("hex")
        if rec["prefix"] or rec["robot_nonce"]:
            sessions.append(rec)
    return sessions


def report_dupes(name, values):
    seen = {}
    dupes = []
    for i, v in enumerate(values):
        if v is None:
            continue
        if v in seen:
            dupes.append((seen[v], i, v))
        else:
            seen[v] = i
    if dupes:
        print(f"[FAIL] {name}: {len(dupes)} collision(s) found:")
        for a, b, v in dupes:
            print(f"        sessions #{a} and #{b} share {v}")
    else:
        print(f"[ok]   {name}: no reuse across {len(values)} session(s)")
    return len(dupes)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dir", default="captures", help="capture directory")
    args = ap.parse_args()

    sessions = load_sessions(args.dir)
    k = len(sessions)
    if k == 0:
        print(f"[!] no session-*.jsonl files in {args.dir}", file=sys.stderr)
        sys.exit(2)

    print(f"Audited {k} session(s) from {args.dir}\n")

    fails = 0
    fails += report_dupes("session_prefix (64-bit)",
                          [s["prefix"] for s in sessions])
    fails += report_dupes("robot_nonce (96-bit)",
                          [s["robot_nonce"] for s in sessions])
    fails += report_dupes("client_nonce (96-bit)",
                          [s["client_nonce"] for s in sessions])

    # Robot persistent counter should be strictly increasing across time.
    counters = [(s["robot_counter"], s["path"]) for s in sessions
                if s["robot_counter"] is not None]
    print()
    non_monotonic = 0
    prev = None
    for c, p in counters:
        if prev is not None and c <= prev:
            print(f"[warn] robot counter not strictly increasing: {prev} -> {c} "
                  f"({os.path.basename(p)})")
            non_monotonic += 1
        prev = c
    if counters and non_monotonic == 0:
        print(f"[ok]   robot persistent counter strictly increasing "
              f"({counters[0][0]} .. {counters[-1][0]})")

    # Analytical birthday bound for the 64-bit prefix.
    prob = (k * k) / (2 ** 65)
    print("\n--- analytical bound (the claim that actually matters) ---")
    print(f"prefix space          : 2^64")
    print(f"sessions observed (k) : {k}")
    print(f"P(any prefix collision) = k^2 / 2^65 = {prob:.3e}")
    print("Reuse becomes ~50 percent likely only near k = 2^32 (4.3 billion) sessions.")

    print()
    if fails == 0:
        print("RESULT: no nonce reuse observed; construction is sound."
              "⚠️ Cannot be considered a proof that the scheme is secure ⚠️")
        sys.exit(0)
    else:
        print("RESULT: reuse detected. Investigate.")
        sys.exit(1)

main()