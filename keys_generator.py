#!/usr/bin/env python3
#
# Unified Ed25519 keys generator for the AIBO TinyConsole.
#
# Both the robot and the client use the same primitive (a raw Ed25519
# keypair), the only difference being in which half is secret and in the
# format of each side.
#
#   python3 keys_generator.py --role robot
#   python3 keys_generator.py --role client
#   python3 keys_generator.py --role symmetric
#
# Dependency: pip install cryptography

import argparse
import os
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey

def c_array(name: str, data: bytes) -> str:
    lines = [f"static const uint8_t {name}[{len(data)}] = {{"]
    for row in range(0, len(data), 8):
        chunk = data[row:row + 8]
        lines.append("    " + ", ".join(f"0x{b:02X}" for b in chunk ) + ",")
    lines.append("};")
    return "\n".join(lines)

def main():
    ap = argparse.ArgumentParser(description="Ed25519 keys generator (robot or client).")
    ap.add_argument("-r", "--role", required=True,
                    choices=["robot", "client", "symmetric"],
                    help="robot/client = Ed25519 identity for that side; "
                         "symmetric = the shared ChaCha20-Poly1305 key "
                         "(pasted into BOTH sides).")
    args = ap.parse_args()

    bar = "=" * 68

    if args.role == "symmetric":
        key = os.urandom(32)

        print(bar)
        print(" Shared ChaCha20-Poly1305 key:")
        print(bar)
        print(c_array("CHACHA_KEY", key))
        print()
        print(bar)
        return

    sk = Ed25519PrivateKey.generate()
    seed = sk.private_bytes_raw()               # 32-byte seed
    pk = sk.public_key().public_bytes_raw()     # 32-byte public key
    combined = seed + pk                         # 64-byte TweetNaCl "SK"

    if args.role == "robot":
        print(bar)
        print(" ConsoleConfig.h  (robot's !secret! key):")
        print(bar)
        print(c_array("ROBOT_ED25519_SK", combined))
        print()
        print(c_array("ROBOT_ED25519_PK", pk))
        print()
        print(bar)
        print(" chacha20_console_client.py  (public key destined for the client):")
        print(bar)
        print(f'ROBOT_PUBKEY_HEX = "{pk.hex()}"')
    else:  # client
        print(bar)
        print(" ConsoleConfig.h  (client's public key):")
        print(bar)
        print(c_array("CLIENT_ED25519_PK", pk))
        print()
        print(bar)
        print(" chacha20_console_client.py  (client's !secret! key):")
        print(bar)
        print(f'CLIENT_PRIVATE_KEY_HEX = "{seed.hex()}"')
        print()
        print(f"# (client public key, for reference: {pk.hex()})")


main()