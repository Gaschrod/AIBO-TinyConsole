#!/usr/bin/env python3
"""
robot_keys_generator.py

Generates the AIBO's persistent Ed25519 identity key pair on any compatible OS (Linux, macOS, Windows). 
The robot itself cannot not generate this key (lack the RNG capabilities).

Should be run once and the resulting secret key written into ConsoleConfig.h and compiled onto the Memory Stick.
To limit the exposure window of the key, it can be rotated by re-running the script.

Usage:
    pip install cryptography
    python3 robot_keys_generator.py

Output:
  1. A C snippet -- ROBOT_ED25519_SK[64] (!secret! key) and ROBOT_ED25519_PK[32] (public key)
    To paste into ConsoleConfig.h
  2. The public key alone, as hex.
"""

from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey


def format_c_array(name: str, data: bytes) -> str:
    lines = [f"static const uint8_t {name}[{len(data)}] = {{"]
    for i in range(0, len(data), 8):
        chunk = data[i:i + 8]
        lines.append("    " + ", ".join(f"0x{b:02X}" for b in chunk) + ",")
    lines.append("};")
    return "\n".join(lines)


def main() -> None:
    sk = Ed25519PrivateKey.generate()
    pk = sk.public_key()

    seed = sk.private_bytes_raw()      # 32 bytes
    pubkey = pk.public_bytes_raw()     # 32 bytes

    # TweetNaCl's "combined" secret-key format is seed || pubkey
    # (64 bytes total) -- exactly what crypto_sign_keypair() would
    # produce from this seed, and what crypto_sign() expects as sk.
    nacl_sk = seed + pubkey
    print("\n")
    print("// ---------------------------------------------------------------")
    print("// Paste into ConsoleConfig.h")
    print("// ---------------------------------------------------------------")
    print("\n")
    print(format_c_array("ROBOT_ED25519_SK", nacl_sk))
    print()
    print(format_c_array("ROBOT_ED25519_PK", pubkey))
    print("// ---------------------------------------------------------------")
    print("\n")
    print("// ---------------------------------------------------------------")
    print("// Distribute this to clients for Trust On First Use (TOFU) verification.")
    print("// ---------------------------------------------------------------")
    print("\n")
    print("robot_ed25519_pubkey_hex =", pubkey.hex())
    print("\n")
main()