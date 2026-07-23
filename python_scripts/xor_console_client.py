#!/usr/bin/env python3
#
# Unauthenticated XOR-stream prototype client for TinyConsole. 
# Performance baseline: does the same TCP request/response as the
# secure client but with a trivial XOR "cipher" and no handshake -> RTT
# difference against chacha20_console_client.py can be used to measure the "cost" 
# of added security
#
# Usage:
#   Interactive:
#     python3 xor_console_client.py --ip 192.168.1.124
#   Latency benchmark:
#     python3 xor_console_client.py --ip 192.168.1.124 \
#             --benchmark --iterations 1000 --warmup 20 --command PING \
#             --csv xor_rtt.csv

import argparse, socket, sys, time, statistics, csv
from typing import Tuple, List

DEFAULT_ROBOT_IP = "192.168.1.124"
DEFAULT_PORT = 7777
XOR_KEY = bytes([0xA5, 0x3C, 0x7F, 0x11, 0xDE])


class XorStream:
    """Continuous XOR keystream; offset is the running byte count"""
    def __init__(self):
        self.offset = 0

    def process(self, data: bytes) -> bytes:
        result = bytearray(len(data))
        for i in range(len(data)):
            result[i] = data[i] ^ XOR_KEY[self.offset % len(XOR_KEY)]
            self.offset += 1
        return bytes(result)


# ---------------------------------------------------------------
#  One timed request/response exchange
# ---------------------------------------------------------------

def exchange(s: socket.socket, tx_stream: XorStream, rx_stream: XorStream,
             plaintext: bytes) -> Tuple[bytes, int]:
    """
    Send one XOR-obfuscated command and read the response.
    Returns (response_plaintext, rtt_ns). The timed window covers
    process -> send -> recv -> process: the full client-observed round trip.
    Responses are tiny (PONG/OK/BYE) so a single recv() captures them whole.
    """
    t0 = time.perf_counter_ns()
    s.sendall(tx_stream.process(plaintext))
    enc = s.recv(1024)
    if not enc:
        raise ConnectionError("Connection closed by robot")
    response = rx_stream.process(enc)
    t1 = time.perf_counter_ns()
    return response, t1 - t0


# ---------------------------------------------------------------
#  Measurement functions (same column format as the AEAD client)
# ---------------------------------------------------------------

def _percentile(sorted_ns: List[int], p: float) -> float:
    n = len(sorted_ns)
    if n == 1:
        return float(sorted_ns[0])
    k = (n - 1) * p
    f = int(k)
    c = min(f + 1, n - 1)
    return sorted_ns[f] + (sorted_ns[c] - sorted_ns[f]) * (k - f)


def summarize(name: str, samples_ns: List[int]) -> None:
    if not samples_ns:
        print(f"\n=== {name}: no samples ===")
        return
    xs = sorted(samples_ns)
    n = len(xs)
    ms = lambda v: v / 1e6
    mean = statistics.fmean(xs)
    stdev = statistics.pstdev(xs) if n > 1 else 0.0
    print(f"\n=== {name}  (n={n}) ===")
    print(f"  min    {ms(xs[0]):9.3f} ms")
    print(f"  mean   {ms(mean):9.3f} ms")
    print(f"  median {ms(_percentile(xs, 0.50)):9.3f} ms")
    print(f"  p95    {ms(_percentile(xs, 0.95)):9.3f} ms")
    print(f"  p99    {ms(_percentile(xs, 0.99)):9.3f} ms")
    print(f"  max    {ms(xs[-1]):9.3f} ms")
    print(f"  stdev  {ms(stdev):9.3f} ms")


def write_csv(path: str, samples_ns: List[int]) -> None:
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["sample", "rtt_us"])
        for i, v in enumerate(samples_ns):
            w.writerow([i, f"{v / 1e3:.3f}"])
    print(f"  wrote {len(samples_ns)} samples to {path}")


def run_benchmark(s: socket.socket, tx_stream: XorStream, rx_stream: XorStream,
                  args: argparse.Namespace) -> None:
    plaintext = (args.command + "\n").encode("utf-8")

    for _ in range(args.warmup):
        exchange(s, tx_stream, rx_stream, plaintext)
        if args.delay:
            time.sleep(args.delay)

    samples: List[int] = []
    for _ in range(args.iterations):
        _, dt = exchange(s, tx_stream, rx_stream, plaintext)
        samples.append(dt)
        if args.delay:
            time.sleep(args.delay)

    summarize(f"RTT '{args.command}' (XOR baseline)", samples)
    if args.csv:
        write_csv(args.csv, samples)

    try:
        s.sendall(tx_stream.process(b"QUIT\n"))
    except Exception:
        pass


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Unauthenticated XOR prototype console client (performance baseline)."
    )
    parser.add_argument("--ip", "-i", default=DEFAULT_ROBOT_IP,
                        help=f"AIBO IP address (default: {DEFAULT_ROBOT_IP})")
    parser.add_argument("--port", "-p", type=int, default=DEFAULT_PORT,
                        help=f"TinyConsole TCP port (default: {DEFAULT_PORT})")
    parser.add_argument("--benchmark", action="store_true",
                        help="Measure round-trip latency instead of the interactive REPL.")
    parser.add_argument("--iterations", type=int, default=1000,
                        help="Number of measured samples (default: 1000).")
    parser.add_argument("--warmup", type=int, default=20,
                        help="Warm-up exchanges discarded before measuring (default: 20).")
    parser.add_argument("--command", default="PING",
                        help="Command to time; use a side-effect-free one like PING (default: PING).")
    parser.add_argument("--delay", type=float, default=0.0,
                        help="Optional seconds to sleep between exchanges (default: 0).")
    parser.add_argument("--csv", default=None,
                        help="Write raw per-sample RTTs (microseconds) to this CSV file.")
    return parser.parse_args()


def main():
    args = parse_args()

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        t0 = time.perf_counter_ns()
        s.connect((args.ip, args.port))
        t_connect = time.perf_counter_ns() - t0
        print(f"Connected to the robot at {args.ip}:{args.port}")
    except Exception as e:
        print(f"Connection error: {e}")
        sys.exit(1)

    # "Handshake" for the XOR prototype = reading the plaintext banner.
    t0 = time.perf_counter_ns()
    banner = b""
    while b"CONSOLE_READY\n" not in banner:
        banner += s.recv(1024)
    t_banner = time.perf_counter_ns() - t0
    print(f"[timing] TCP connect: {t_connect/1e6:.3f} ms | "
          f"banner read: {t_banner/1e6:.3f} ms")

    # One stream for sending, one for receiving (offsets advance independently).
    tx_stream = XorStream()
    rx_stream = XorStream()

    if args.benchmark:
        try:
            run_benchmark(s, tx_stream, rx_stream, args)
        finally:
            s.close()
        return

    try:
        while True:
            try:
                cmd = input("AIBO> ")
            except EOFError:
                break
            if not cmd:
                continue

            plaintext = (cmd + "\n").encode("utf-8")
            if cmd.strip().upper() == "QUIT":
                s.sendall(tx_stream.process(plaintext))
                break
            try:
                response, dt = exchange(s, tx_stream, rx_stream, plaintext)
                print("ROBOT:", response.decode("utf-8", errors="replace").strip(),
                      f"  [rtt {dt/1e6:.3f} ms]")
            except ConnectionError:
                print("The robot ended the connection")
                break

    except KeyboardInterrupt:
        print("\nEnding connection...")
    finally:
        s.close()


main()
