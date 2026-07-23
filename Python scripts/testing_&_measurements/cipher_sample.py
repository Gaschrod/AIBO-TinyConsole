#!/usr/bin/env python3
"""
Collects every 256-byte ciphertext block from the captured session logs and
writes them concatenated to a single binary file, then (if ent is installed),
prints entropy/chi-square/arithmetic-mean/Monte-Carlo pi/serial-correlation. 
Also possible, for further analysis, to feed the output to dieharder or NIST STS.

Should be used as a cheap regression/sanity check:
Would flag a gross implementation error or keystream reuse, an accidental ECB-like mode, a stuck RNG, or padding that leaks structure onto the wire.

Please note: frames are fixed 256-byte padded blocks with a 16-byte Poly1305 tag
If the tag or header was included in the sample, it would mix with non-ciphertext bytes. 
This script samples ciphertext *ONLY* -> stats will only target AEAD construction

Usage:
    python3 cipher_sample.py --dir captures --out cipher_sample.bin
    ent cipher_sample.bin                 # if not auto-run
"""

import argparse
import glob
import json
import os
import shutil
import subprocess
import sys


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dir", default="captures", help="capture directory")
    ap.add_argument("--out", default="cipher_sample.bin", help="output binary")
    args = ap.parse_args()

    blocks = 0
    total = 0
    with open(args.out, "wb") as out:
        for path in sorted(glob.glob(os.path.join(args.dir, "session-*.jsonl"))):
            with open(path) as f:
                for line in f:
                    try:
                        ev = json.loads(line)
                    except ValueError:
                        continue
                    if ev.get("event") == "frame" and "ct_hex" in ev:
                        data = bytes.fromhex(ev["ct_hex"])
                        out.write(data)
                        blocks += 1
                        total += len(data)

    if blocks == 0:
        print(f"[!] no ciphertext frames found in {args.dir}. Run some traffic "
              f"through aibo_mitm.py --mode passthrough first.", file=sys.stderr)
        sys.exit(2)

    print(f"[*] wrote {total} bytes ({blocks} ciphertext blocks) to {args.out}")
    if total < 100_000:
        print("[i] Note: this is a small sample. `ent` is fine here, but dieharder "
              "and NIST STS need many megabytes to be meaningful.")

    ent = shutil.which("ent")
    if ent:
        print(f"\n[*] running: ent {args.out}\n")
        subprocess.run([ent, args.out])
    else:
        print("\n[i] `ent` not installed. On Arch:  sudo pacman -S ent")
        print(f"    Then run:  ent {args.out}")
        print(f"    Or:        dieharder -a -f {args.out}   (needs a large sample)")

main()