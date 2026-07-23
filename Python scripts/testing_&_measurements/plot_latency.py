#!/usr/bin/env python3
"""
plot_latency.py
Turn the RTT CSVs produced by the benchmark clients into a figure and a
LaTeX summary table.

Usage:
    python3 plot_latency.py --xor xor_rtt.csv --aead aead_rtt.csv \
            --out latency_hist.png --tex latency_table.tex

Dependencies:
    pip install matplotlib
"""

import argparse
import csv
import statistics
import sys
from typing import List, Tuple

import matplotlib
matplotlib.use("Agg")            # headless: write files, no display needed
import matplotlib.pyplot as plt


# ----------------------------------------------------------------------
#  Data loading
# ----------------------------------------------------------------------

def load_rtt_us(path: str) -> List[float]:
    """Read a benchmark CSV and return RTTs in microseconds.

    Accepts a 'sample,rtt_us' header or bare rows; also tolerates a single
    column of RTTs.
    """
    values: List[float] = []
    with open(path, newline="") as f:
        for row in csv.reader(f):
            if not row:
                continue
            cell = row[-1].strip()          # rtt is the last column
            try:
                values.append(float(cell))
            except ValueError:
                continue                    # header or junk line
    if not values:
        raise ValueError(f"no numeric samples found in {path}")
    return values


# ----------------------------------------------------------------------
#  Statistics
# ----------------------------------------------------------------------

def _percentile(sorted_vals: List[float], p: float) -> float:
    n = len(sorted_vals)
    if n == 1:
        return sorted_vals[0]
    k = (n - 1) * p
    f = int(k)
    c = min(f + 1, n - 1)
    return sorted_vals[f] + (sorted_vals[c] - sorted_vals[f]) * (k - f)


# metric key -> (label, is_integer)
METRICS = [
    ("n",      "Samples",     True),
    ("min",    "Min (ms)",    False),
    ("mean",   "Mean (ms)",   False),
    ("median", "Median (ms)", False),
    ("p95",    "p95 (ms)",    False),
    ("p99",    "p99 (ms)",    False),
    ("max",    "Max (ms)",    False),
    ("stdev",  "Std dev (ms)", False),
]


def stats_ms(rtt_us: List[float]) -> dict:
    """Return summary statistics, converted to milliseconds."""
    xs = sorted(v / 1000.0 for v in rtt_us)     # us -> ms
    n = len(xs)
    return {
        "n":      n,
        "min":    xs[0],
        "mean":   statistics.fmean(xs),
        "median": _percentile(xs, 0.50),
        "p95":    _percentile(xs, 0.95),
        "p99":    _percentile(xs, 0.99),
        "max":    xs[-1],
        "stdev":  statistics.pstdev(xs) if n > 1 else 0.0,
    }


# ----------------------------------------------------------------------
#  Plot
# ----------------------------------------------------------------------

def make_histogram(series: List[Tuple[str, List[float]]],
                   out_path: str, bins: int, clip_pct: float,
                   title: str) -> None:
    fig, ax = plt.subplots(figsize=(7.2, 4.2))

    # Display range: clip to a high percentile so a few outliers don't
    # squash the bulk of the distribution (all data still counted).
    xmax = 0.0
    for _, rtt in series:
        xs = sorted(v / 1000.0 for v in rtt)
        xmax = max(xmax, _percentile(xs, clip_pct / 100.0))
    if xmax <= 0:
        xmax = None

    for label, rtt in series:
        xs = [v / 1000.0 for v in rtt]                  # ms
        st = stats_ms(rtt)
        ax.hist(xs, bins=bins, range=(0, xmax) if xmax else None,
                alpha=0.55, histtype="stepfilled", linewidth=1.2,
                label=f"{label} (median {st['median']:.2f} ms)")
        ax.axvline(st["median"], linestyle="--", linewidth=1.0, alpha=0.9)

    ax.set_xlabel("Round-trip latency (ms)")
    ax.set_ylabel("Number of samples")
    if xmax:
        ax.set_xlim(0, xmax)
    if title:
        ax.set_title(title)
    ax.grid(True, axis="y", alpha=0.3)
    ax.legend(frameon=False)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    # Drop a PNG for preview.
    print(f"wrote {out_path}")
    plt.close(fig)


# ----------------------------------------------------------------------
#  LaTeX + console tables
# ----------------------------------------------------------------------

def _tex_escape(s: str) -> str:
    for a, b in (("\\", r"\textbackslash{}"), ("&", r"\&"), ("%", r"\%"),
                 ("_", r"\_"), ("#", r"\#"), ("$", r"\$"), ("{", r"\{"),
                 ("}", r"\}"), ("~", r"\textasciitilde{}"),
                 ("^", r"\textasciicircum{}")):
        s = s.replace(a, b)
    return s


def _fmt(key: str, is_int: bool, val: float) -> str:
    return f"{int(val)}" if is_int else f"{val:.3f}"


def latex_table(series_stats: List[Tuple[str, dict]], caption: str,
                label: str) -> str:
    n = len(series_stats)
    add_delta = (n == 2)
    ncols = 1 + n + (1 if add_delta else 0)
    colspec = "l" + "r" * (n + (1 if add_delta else 0))

    head = ["Metric"] + [_tex_escape(lbl) for lbl, _ in series_stats]
    if add_delta:
        head.append(r"$\Delta$")

    lines = [
        r"\begin{table}[ht]",
        r"  \centering",
        f"  \\caption{{{_tex_escape(caption)}}}",
        f"  \\label{{{label}}}",
        f"  \\begin{{tabular}}{{{colspec}}}",
        r"    \toprule",
        "    " + " & ".join(head) + r" \\",
        r"    \midrule",
    ]

    for key, mlabel, is_int in METRICS:
        cells = [mlabel]
        for _, st in series_stats:
            cells.append(_fmt(key, is_int, st[key]))
        if add_delta:
            if is_int:
                cells.append("")
            else:
                d = series_stats[1][1][key] - series_stats[0][1][key]
                cells.append(f"{d:+.3f}")
        lines.append("    " + " & ".join(cells) + r" \\")

    lines += [r"    \bottomrule", r"  \end{tabular}"]

    if add_delta:
        f0 = series_stats[0][1]["mean"]
        f1 = series_stats[1][1]["mean"]
        if f0 > 0:
            factor = f1 / f0
            lines.append(
                r"  \par\smallskip"
                f"\n  \\footnotesize $\\Delta$ = "
                f"{_tex_escape(series_stats[1][0])} $-$ "
                f"{_tex_escape(series_stats[0][0])}; "
                f"mean overhead {f1 - f0:+.3f}~ms "
                f"($\\times${factor:.2f}).")

    lines.append(r"\end{table}")
    return "\n".join(lines) + "\n"


def print_console_table(series_stats: List[Tuple[str, dict]]) -> None:
    labels = [lbl for lbl, _ in series_stats]
    w = max(12, *(len(l) for l in labels))
    header = "Metric".ljust(12) + "".join(l.rjust(w + 2) for l in labels)
    print("\n" + header)
    print("-" * len(header))
    for key, mlabel, is_int in METRICS:
        row = mlabel.ljust(12)
        for _, st in series_stats:
            row += _fmt(key, is_int, st[key]).rjust(w + 2)
        print(row)
    if len(series_stats) == 2:
        m0 = series_stats[0][1]["mean"]
        m1 = series_stats[1][1]["mean"]
        print(f"\nMean overhead: {m1 - m0:+.3f} ms"
              + (f"  (x{m1 / m0:.2f})" if m0 > 0 else ""))


# ----------------------------------------------------------------------
#  CLI
# ----------------------------------------------------------------------

def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--xor", metavar="CSV", help="XOR baseline RTT CSV.")
    p.add_argument("--aead", metavar="CSV", help="ChaCha20-Poly1305 RTT CSV.")
    p.add_argument("--series", action="append", default=[], metavar="LABEL=CSV",
                   help="Extra labelled series (repeatable).")
    p.add_argument("--out", default="latency_hist.png",
                   help="Output figure path (.png recommended; default: latency_hist.png).")
    p.add_argument("--tex", default="latency_table.tex",
                   help="Output LaTeX table path (default: latency_table.tex).")
    p.add_argument("--bins", type=int, default=60, help="Histogram bins (default: 60).")
    p.add_argument("--clip", type=float, default=99.5,
                   help="Clip the x-axis at this percentile for readability (default: 99.5; use 100 to disable).")
    p.add_argument("--title", default="", help="Optional figure title.")
    p.add_argument("--caption",
                   default="Round-trip latency of one PING command over the TinyConsole link.",
                   help="LaTeX table caption.")
    p.add_argument("--tex-label", default="tab:latency", help="LaTeX table label.")
    return p.parse_args()


def main() -> None:
    args = parse_args()

    series: List[Tuple[str, List[float]]] = []
    if args.xor:
        series.append(("XOR baseline", load_rtt_us(args.xor)))
    if args.aead:
        series.append(("ChaCha20-Poly1305", load_rtt_us(args.aead)))
    for spec in args.series:
        if "=" not in spec:
            sys.exit(f"--series expects LABEL=CSV, got: {spec}")
        label, path = spec.split("=", 1)
        series.append((label.strip(), load_rtt_us(path.strip())))

    if not series:
        sys.exit("No input. Pass --xor/--aead or --series LABEL=CSV.")

    series_stats = [(lbl, stats_ms(rtt)) for lbl, rtt in series]

    print_console_table(series_stats)

    make_histogram(series, args.out, args.bins, args.clip, args.title)

    tex = latex_table(series_stats, args.caption, args.tex_label)
    with open(args.tex, "w") as f:
        f.write(tex)
    print(f"wrote {args.tex}")


main()
