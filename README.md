# AIBO-TinyConsole

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![ROS2](https://img.shields.io/badge/ROS%202-Jazzy%20Jalisco-22314E)
![Platform](https://img.shields.io/badge/platform-Aperios%20%2F%20OPEN--R-lightgrey)
![Status](https://img.shields.io/badge/status-research%20%2F%20thesis-yellow)

**A minimal, retrofitted encrypted control channel for the Sony AIBO ERS-7 — and the reference case study for a master's thesis on securing legacy systems in OT environments.**

The main elements of this repository are:

1. **Secure_TinyConsole** — a small OPEN-R object that runs *on* the AIBO ERS-7 and exposes an encrypted, mutually-authenticated TCP console for issuing motion commands.
2. **chacha20_console_client.py** — the ChaCha20-Poly1305 / Ed25519 python equivalent to the OPEN-R object, intended to be run on the remote client.
3. **`aibo_bridge`** — a ROS 2 package that wraps the protocol client and exposes the AIBO as a normal ROS 2 node (topics for commands, `/cmd_vel` teleop, keyboard/joystick teleop nodes).
4. The accompanying **master's thesis** (PDF), which uses this implementation as its case study.

> If you just want to drive the robot from ROS 2, skip to [Quick Start](#quick-start). If you want the "why", read [What is this, and why does it exist?](#what-is-this-and-why-does-it-exist).

---

## Table of Contents

- [What is this, and why does it exist?](#what-is-this-and-why-does-it-exist)
- [Architecture at a glance](#architecture-at-a-glance)
- [Repository layout](#repository-layout)
- [Security design summary](#security-design-summary)
- [Prerequisites](#prerequisites)
- [Setting up the OPEN-R SDK](#setting-up-the-open-r-sdk)
- [Building and flashing the robot firmware](#building-and-flashing-the-robot-firmware)
- [Generating key material](#generating-key-material)
- [Setting up the ROS 2 bridge](#setting-up-the-ros-2-bridge)
- [Quick Start](#quick-start)
- [Usage](#usage)
- [Protocol overview](#protocol-overview)
- [Threat model & known limitations](#threat-model--known-limitations)
- [Roadmap](#roadmap)
- [Hardware safety](#hardware-safety)
- [Academic context](#academic-context)
- [Third-party components & attribution](#third-party-components--attribution)
- [License](#license)

---

## What is this, and why does it exist?

Legacy systems are numerous in Operational Technology (OT) environments — factories, labs, warehouses — which are full of embedded devices that either were never designed with modern network security in mind or are just too old to be considered secure anymore. 
Even more, they sometimes the vendors who built them. On occasion, it is simply not possible to replace them (because of their cost, because of the fact that they are too deeply rooted in their environment and often also because they are considered "fine" as in "they haven't broken yet").

This thesis uses the **Sony AIBO ERS-7** as a concrete, physical stand-in for that class of problem:

- It runs **Aperios/OPEN-R** on a **MIPS R7000**, cross-compiled with a **GCC v3.3.2** toolchain — a genuinely obsolete, vendor-abandoned embedded platform.
- It has **no usable RNG** at the OS level (`gettimeofday`, `clock`, `time` are non-functional under newlib on this platform), which rules out a lot of "just do it the normal way" cryptographic advice.
- It has real physical actuators, so a security bug isn't just "data gets leaked" — it can also mean unsafe motion or a bricked robot.
- It is, as far as this work is aware, previously undocumented from a security-research standpoint.

The thesis argues — and this repository is the proof of concept for — a **minimal retrofit** approach: instead of redesigning the platform's communication stack from scratch, add a small, self-contained, strongly-authenticated encrypted channel *alongside* the existing system (the word ‘system’ is used here to refer to the Aperios OS and not to the MIND personalities for which it has not been tested whether MIND and TinyConsole can run alongside one another), adapting the robot "as is." The thesis includes an explicit "why not redesign the protocol" discussion for readers who want the counter-arguments.

**TinyConsole** is that channel: a small OPEN-R object exposing a command console over TCP, secured with AEAD encryption and mutual signature-based authentication, built entirely within the constraints above. **`aibo_bridge`** then makes that channel usable from a modern robotics stack (ROS 2), so the secured legacy robot can participate in normal robotics workflows (teleop, `/cmd_vel`, etc.) instead of being retired.

The broader argument (developed in the thesis, not fully reproduced here) is ecological as much as technical: extending the working life of hardware that still functions mechanically, rather than treating "unsupported firmware" as equivalent to "e-waste."

---

## Architecture at a glance

```
 PC / ROS 2 Jazzy (Ubuntu 24.04)                              Sony AIBO ERS-7 (Aperios / OPEN-R)
┌──────────────────────────────┐                              ┌───────────────────────────────────┐
│                              │   Encrypted TCP, port 7777   │                                   │
│   aibo_teleop_key /          │   ChaCha20-Poly1305 AEAD     │                                   │
│   aibo_teleop_joy            │   mutual Ed25519 handshake   │                                   │
│        │                     │◄────────────────────────────►│      TinyConsole (OPEN-R object)  │
│        ▼                     │                              │      - handshake + AEAD frames    │
│   /cmd_vel, ~/command  ──►   │                              │      - motion state machine       │
│                              │                              │            │                      │
│   aibo_bridge_node           │                              │            ▼                      │
│   (AiboLink, Python/         │                              │   OPEN-R joint commands (12 legs  │
│    `cryptography`)           │                              │   joints), PID-gained actuators   │
│        │                     │                              │                                   │
│        ▼                     │                              │                                   │
│   ~/response, ~/connected    │                              │                                   │
└──────────────────────────────┘                              └───────────────────────────────────┘
```

The TinyConsole object and the ROS 2 bridge are independent of each other over the wire — the bridge is just one possible client. The standalone `tools/chacha20_console_client.py` script talks the same protocol without needing ROS 2 at all, which is useful for bring-up and debugging.

---

## Repository layout

```
AIBO-TinyConsole/
└── OPEN-R_SDK # Sony official SDK files (unmodified)
│     ├── normal_size_screen.sh # a bash script to easily add 1920x1080 screen resolution in Ubuntu 8.04
│     └── patch_file.txt # necessary apt in order to make the SDK work
│
├── XOR_TinyConsole # naive implementation of the console which only uses XOR encryption, no movements and only supports "PING" and "QUIT" commands
├── aibo_bridge/ # ros 2 implementation (run on Ubuntu 24.04 or equivalent)
│     ├── aibo_bridge/
│     |   ├── __init__.py
│     |   ├── aibo_bridge_node.py  # ROS 2 node wrapping AiboLink
│     |   ├── aibo_link.py         # low-level encrypted TCP link (protocol client)
│     |   ├── aibo_teleop_joy.py   # PS4/DualShock joystick teleop
│     |   ├── aibo_teleop_key.py   # keyboard teleop
│     |   └── replay_watermark.py  # client-side rollback protection
│     │
│     ├── launch/
│     │   ├── aibo_bridge_launch.py
│     │   └── aibo_teleop_joy_launch.py
│     │
│     ├── resource/
│     ├── package.xml
│     ├── setup.cfg
│     └── setup.py
│
├── c20p1305/ # pointer to Will Glozer's implementation of ChaCha20+Poly1305
├── config_files/ # .CFG files needed in the OPEN-R filesystem on the AIBO's Memory Stick
├── docs/ # various relevant documentations
│    ├── OPEN-R_docs/ # all Sony's official documentation related to OPEN-R
│    ├── Sony_official_docs/ # other docs made by Sony related to the ERS-7 and alike
│    └── other_relevant_docs/ # docs on how to program using the OPEN-R, ROS2 and URBI
│
├── measurements/ # various measurements related to RTT and the cryptographic overhead added by TinyConsole
│    └──  CSV
│
├── motion_files/ # files used to create movements in TinyConsole (mainly FORWARD and BACKWARD which was very hard to get right without those files)
├── python_scripts/ # various important scripts
│    ├── testing_&_measurements/ # scripts used to generate csv, measurements and to test the implementation
│    ├── chacha20_console_client.py # secure script used to connect to TinyConsole when running on AIBO
│    ├── keys_generator.py # used to generate the various keys used to secure the communication
│    ├── replay_watermak.py # script used by chacha20_console_client.py in order to prevent rollback attack
│    └── xor_console_client.py # initial naive implementation using only XOR, must be used with XOR_TinyConsole (runs on AIBO)
│
├── secure_TinyConsole/                       # robot side — must be compiled with the OPEN-R SDK toolchain
│    ├── Makefile
|    └── TinyConsole/ # necessary files for TinyConsole, refer to the master thesis/OPEN-R documentation for more details on which file does what
│
├── thesis_defense_slides # slides that were used during the thesis defense (pdf & pptx)
├── LICENSE # MIT license
├── README.md # the file you're reading
└── The Man-Bites-Dog Phenomenon in Cybersecurity - A Case Study on Hardening Legacy Hardware Communications with the Sony AIBO ERS-7.pdf # Master Thesis
```

---

## Security design summary

TinyConsole is not "TLS on a robot." It's a purpose-built protocol shaped by the platform's constraints:

- **AEAD encryption** — ChaCha20-Poly1305 (RFC 7539 framing) protects every command/response frame after the handshake, with fixed-size padded frames to reduce traffic-analysis leakage.
- **Mutual authentication, not just encryption** — both sides prove their identity with **Ed25519** signatures (via TweetNaCl on the robot, `cryptography` on the PC side) over a shared handshake transcript. Deterministic EdDSA signing was chosen specifically because it needs **no runtime randomness**, satisfying the platform's no-RNG constraint.
- **Pinned, never-transmitted keys** — public keys are provisioned out-of-band on both ends, not exchanged on the wire. Transmitting a "trusted" public key at connect time would let an attacker simply substitute their own; pinning removes that failure mode.
- **Anti-replay / anti-rollback** — a persistent, CRC-validated session counter (stored redundantly on the Memory Stick) is embedded in the signed handshake transcript, so a rolled-back or cloned robot identity is detectable by the client.
- **Fail-closed throughout** — missing keys, a corrupt replay counter, a failed persist, or a bad signature all abort the connection rather than falling back to something weaker.

See [`ConsoleConfig.h`](secure_TinyConsole/TinyConsole/ConsoleConfig.h) and the thesis for the full rationale, including the primitives that were evaluated and *not* used (e.g. Ascon-AEAD128, documented as a deliberate "road not taken").

---

## Prerequisites

**To build and flash the robot firmware:**
- A Sony AIBO ERS-7 (or ERS-7M2/M3) with a compatible Memory Stick (i.e a Memory Stick that was modified using StikZap via a Sony CLIé or a Pink PMS).
- The **Sony OPEN-R SDK**.
- A Linux environment able to run the SDK's `mipsel-linux-g++`/`mipsel-linux-gcc` (GCC v3.3.2) cross-compiler. For the thesis, an Ubuntu 8.04 LTS VM (guest) running in VMWare on Windows 11 (host) was used. It needed some adaptions which are not discussed in the thesis nor in this documentation (updating the apt list was one of those adaptions).

**To build and run the ROS 2 bridge:**
- Ubuntu 24.04 with **ROS 2 Jazzy Jalisco** (if you want to mirror what I did, use this in a VM but another Linux distro supported by ROS 2 should work)/
- Python 3 with the [`cryptography`](https://pypi.org/project/cryptography/) package (`python3-cryptography`).
- The `joy` package if you want the PS4/DualShock teleop (`sudo apt install ros-jazzy-joy`).

**To just try the protocol without ROS 2:**
- Python 3 + `cryptography` — that's it (`python_scripts/chacha20_console_client.py` has no other dependencies).

---

## Setting up the OPEN-R SDK

The OPEN-R SDK is Sony's proprietary toolchain for AIBO development; this README isn't a substitute for its own documentation. The official manuals are included for reference under [`docs/OPEN-R_docs/`](docs/OPEN-R_docs/):

- `InstallationGuide_E.pdf` — SDK installation itself.
- `ProgrammersGuide_E.pdf` — OPEN-R object model, subjects/observers, the build pipeline (`stubgen2`, `mkbin`).
- `Level2ReferenceGuide_E.pdf` — lower-level API reference (joints, primitives, memory regions).
- `InternetProtocolVersion4_E.pdf` — the TCP/IP stack (`antStackRef`, `TCPEndpointMsg`, etc.) that TinyConsole builds on.
- `ModelInformation_210_E.pdf` — the ERS-210 official documentation (joints limitations mostly).
- `ModelInformation_220_E.pdf` — the ERS-220 official documentation (joints limitations mostly).
- `ModelInformation_7_E.pdf` — the ERS-7 official documentation (joints limitations mostly).

A few points that aren't obvious from the manuals and are worth knowing going in:

- Set `OPENRSDK_ROOT` to point at your SDK install before building — the [`Makefile`](secure_TinyConsole/TinyConsole/Makefile) reads it directly (defaults to `/usr/local/OPEN_R_SDK`).
- The toolchain rejects **non-ASCII characters** in source files outright — no smart quotes, en-dashes, arrows, etc., anywhere in `.c`/`.cc`/`.h` files, or you'll get cascading parse errors far from the actual offending character.
- This is an old, Makefile-based build system; don't expect it to play nicely with CMake/C99-oriented tooling.

---

## Building and flashing the robot firmware

```bash
cd secure_TinyConsole
export OPENRSDK_ROOT=/path/to/OPEN_R_SDK      # if not already set / not the default location
make                                          # produces tinyConsole.bin
make install                                  # gzips it into ../MS/OPEN-R/MW/OBJS/TINYCONS.BIN
...
make clean                                    # removes compiled files, necessary if you want to build again the binary if you made modifications
```

`make install` writes into a local `MS/` staging directory (override with `INSTALLDIR=`), mirroring the OPEN-R Memory Stick layout. Copy the resulting `MS/OPEN-R` tree onto the AIBO's actual Memory Stick alongside your `OPEN-R` configuration (BASIC, WLAN or WCONSOLE), following the SDK's own instructions for registering an OPEN-R object to run at boot.

`rfc7539_test.c` (in `secure_TinyConsole/TinyConsole/`) is a standalone host-side test of the AEAD implementation against the RFC 7539 test vectors — useful for sanity-checking the crypto primitives with a normal host compiler before cross-compiling for the robot.

---

## Generating key material

Both the robot's and the client's identities are generated **offline**, once, with [`python_scripts/keys_generator.py`](python_scripts/keys_generator.py):

```bash
python3 python_scripts/keys_generator.py --role robot       # robot's Ed25519 identity
python3 python_scripts/keys_generator.py --role client       # client's Ed25519 identity
python3 python_scripts/keys_generator.py --role symmetric    # shared ChaCha20-Poly1305 key
```

Each invocation prints exactly what goes where:

| Generated for | Goes into | Notes |
|---|---|---|
| Robot secret key | `ConsoleConfig.h` → `ROBOT_ED25519_SK` | **Secret.** Never leaves the firmware. |
| Robot public key | `ConsoleConfig.h` (`ROBOT_ED25519_PK`) *and* the PC-side config | Pinned on the client so it can verify the robot. |
| Client public key | `ConsoleConfig.h` → `CLIENT_ED25519_PK` | Pinned on the robot so it can verify the client. |
| Client secret key | PC-side config (`aibo_keys.yaml` / client script) | **Secret.** Never commit it. |
| Shared ChaCha key | `ConsoleConfig.h` and PC-side config | Symmetric — treat as replicated-but-sensitive, *not* as an identity proof (that's what the Ed25519 keys are for). |

After regenerating keys, rebuild and reflash the firmware (see above) so the robot's copy matches.

---

## Setting up the ROS 2 bridge

```bash
cd aibo_bridge
colcon build --packages-select aibo_bridge
source install/setup.bash
```

Create your own (uncommitted) secrets file based on `aibo_bridge/config/aibo_keys.yaml`:

```yaml
aibo_bridge:
  ros__parameters:
    chacha_key: "<64 hex chars>"
    robot_ed25519_pubkey: "<64 hex chars>"
    client_ed25519_seed: "<64 hex chars>"
```

The node **fails closed and refuses to start** if any of these three are missing — they're treated as a security control, not a convenience default with a fallback.

---

## Quick Start

```bash
# 1. Sanity-check the link without ROS 2 at all:
python3 python_scripts/chacha20_console_client.py --ip 192.168.1.124

# 2. Or launch the full ROS 2 bridge:
ros2 launch aibo_bridge aibo_bridge_launch.py \
    params_file:=/path/to/your/aibo_keys.yaml \
    robot_ip:=192.168.1.124

# 3. Drive it:
ros2 launch aibo_bridge aibo_teleop_joy_launch.py     # PS4 controller
# — or —
ros2 run aibo_bridge aibo_teleop_key                  # keyboard
```

---

## Usage

### Commands understood by TinyConsole

| Command | Effect | Response |
|---|---|---|
| `PING` | Liveness check | `PONG` |
| `GET_UP` | Stand up (broadbase pose) | `STANDING_UP` |
| `REST` | Lie down (sleeping pose) | `RESTING` |
| `FORWARD` | Start forward trot (loops until interrupted) | `MOVING_FORWARD` |
| `BACK` | Start backward trot | `MOVING_BACK` |
| `STOP` | Stop, return to broadbase | `STOPPING` |
| `HELP` | List available commands | — |
| `INFO` | Robot self-description | — |
| `QUIT` | Close the connection | `BYE` |
| Anything else | Unsupported command | `OK` |

Motion commands return immediately; the actual motion runs asynchronously via the OPEN-R scheduler's `Ready()` callback loop, driven by a small interpolated-pose state machine (sleeping → rising → broadbase, and a diagonal-trot gait for walking).

### ROS 2 topics (`aibo_bridge_node`)

| Topic | Type | Direction | Purpose |
|---|---|---|---|
| `~/command` | `std_msgs/String` | subscribe | Pass any TinyConsole command straight through. |
| `/cmd_vel` | `geometry_msgs/Twist` | subscribe | `linear.x` mapped to `FORWARD`/`BACK`/`STOP` (deadband-filtered, debounced). |
| `~/response` | `std_msgs/String` | publish | The AIBO's plaintext response to the last command. |
| `~/connected` | `std_msgs/Bool` | publish | Whether the encrypted TCP link is currently up. |

Key node parameters: `robot_ip`, `robot_port`, `chacha_key`, `robot_ed25519_pubkey`, `client_ed25519_seed`, `cmd_vel_deadband`, `reconnect_period`, `motion_timeout`. The bridge also locks ROS 2 DDS discovery to `localhost` by default (`ROS_AUTOMATIC_DISCOVERY_RANGE`), so nothing else on the network segment can inject `/cmd_vel`/`~/command` traffic without deliberately opting out of that.

---

## Protocol overview

At a high level, one session looks like this:

```
Client                                              Robot (TinyConsole)
  |  TCP connect                                            |
  |<---------------- banner + 12-byte nonce ----------------|
  |----------------- 12-byte client nonce ----------------->|
  |<---------- 64-byte Ed25519 signature (robot) -----------|   (verify against pinned robot pubkey)
  |------------ 64-byte Ed25519 signature (client) -------->|   (robot verifies against pinned client pubkey)
  |==============  AEAD session established  ===============|
  |------ [len][ciphertext][16-byte Poly1305 tag] --------->|   commands
  |<----- [len][ciphertext][16-byte Poly1305 tag] ----------|   responses
```

Every AEAD frame carries a **fixed-size padded plaintext block**, so frame sizes don't leak command length. The 12-byte nonce is built from a persistent session counter, the client's IP, and a per-message counter, with TX/RX separated by a high bit.

This section is intentionally a summary, not a specification — for the full protocol rationale (nonce construction, why HChaCha20-derived session keys, the no-RNG workaround, etc.), see the [`thesis`](The_Man-Bites-Dog_Phenomenon_in_Cybersecurity.pdf).

---

## Threat model & known limitations

**What the protocol defends against:**
- Passive eavesdropping on the command channel.
- On-path tampering or injection (AEAD authentication fails closed).
- Impersonation of either endpoint (mutual, pinned-key Ed25519 authentication).
- Replay of a captured session, and rollback of the robot's identity/session state (persistent, CRC-checked counter).

**Known, accepted limitations (also discussed in the thesis):**
- The handshake banner, nonces, and signatures are sent in the clear by design — they're not secrets — but this means a passive observer can link sessions to the same robot identity over time.
- The symmetric ChaCha key is shared across all legitimate clients; it provides confidentiality, not identity. Identity assurance comes entirely from the Ed25519 keys.
- No forward secrecy yet — a compromised long-term key can decrypt past traffic if it was also captured. See [Roadmap](#roadmap).
- This is research/thesis code for a single, specific, obsolete embedded platform — it has not been through independent security review and should not be treated as a general-purpose IoT security library.

---

## Roadmap
This is only intended as a general overview of potential future works and absolutely not a guarantee that I will ever implement any of it (but it would be cool).

If you wish to build on top of it, feel free to fork the repository or to send me an email at 9krkyc1ts@mozmail.com
- **More movements/better integration with existing AIBO remote control** — as of now, remote control is quite limited (no left/right turn)
-**Secure camera streaming** — as the name says, the AIBO supports direct camera streaming (with the limited FPS of the camera), using this stream *inside* of TinyConsole could be a cool addition
- **X25519 static-ephemeral key exchange** — identified as the intended upgrade path for forward secrecy; not yet implemented.
- **TOFU-style client-side key pinning** — to ease first-contact key provisioning without weakening the pinning model.
- **Ascon-AEAD128** was evaluated as an alternative AEAD primitive (better fit for a multiply-light, SIMD-less scalar core) and deliberately *not* adopted for this platform — no prior MIPS porting precedent, no cross-library validation path available at the time, and standard-finalization timing risk. Documented in the thesis as a considered-and-rejected option, not an oversight.

---

## Hardware safety

> **Read before modifying any pose table or gain values.** The AIBO ERS-7's joint limits are **asymmetric between left and right legs** (see the official ERS-7 Model Information documentation for exact values), and joint gains (`SetJointGain`) must only be applied *after* `OpenPrimitives()` has populated the joint IDs — applying them earlier silently applies gains to uninitialized joints. Pushing a joint outside its physical range, or moving with unset gains, can damage the robot's motors/legs. If you're editing the pose tables in `TinyConsole.cc`, cross-check every value against Sony's official joint-limit documentation first.

---

## Academic context

This repository is the software artifact accompanying a master's thesis on securing legacy systems network communications using the Sony AIBO ERS-7 as the case study.

> Regardin, A. (2026). *The Man-Bites-Dog Phenomenon in Cybersecurity - A Case Study on Hardening Legacy Hardware Communications with the Sony AIBO ERS-7* (Master's thesis). Université libre de Bruxelles (ULB).
> Full text: [`The_Man-Bites-Dog_Phenomenon_in_Cybersecurity.pdf`](The_Man-Bites-Dog_Phenomenon_in_Cybersecurity.pdf)
> 
---

## Third-party components & attribution

The `secure_Tinyconsole/TinyConsole` directory includes several third-party cryptographic implementations, kept largely unmodified and each under its own terms:

- **ChaCha20** (`chacha_merged.c`) — D. J. Bernstein, released into the public domain.
- **TweetNaCl** (`tweetnacl.c`/`.h`) — public-domain Ed25519/Curve25519/Salsa20 implementation.
- **poly1305-donna** — public-domain Poly1305 implementation (32-bit variant used here for MIPS compatibility).
- The eSTREAM `ecrypt-*` portability headers, used as the scaffolding around the ChaCha20 core.

These retain their original public-domain status regardless of the license below.

---

## License

This project (excluding the third-party components listed above, and the bundled Sony OPEN-R SDK documentation, which remains Sony's own copyrighted material provided for reference only) is released under the **MIT License**. See [`LICENSE`](LICENSE) for the full text.
