#!/usr/bin/env python3
import csv
import os
import re
import subprocess
import sys

PROGRAM   = "./test"
OPTIONS   = ["l", "v", "b", "p"]
CORES     = [0, 1, 2, 3]
GROUP     = 10
OUT       = "results.csv"

DOUBLE_RE = re.compile(r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?")

def pin(core):
    return lambda: os.sched_setaffinity(0, {core})

def parse_average(text, label):
    vals = [float(x) for x in DOUBLE_RE.findall(text)]
    n = len(vals) // GROUP

    return [sum(vals[i*GROUP:(i+1)*GROUP]) / GROUP for i in range(n)]

def main():
    procs = {}
    for opt, core in zip(OPTIONS, CORES):
        procs[opt] = subprocess.Popen(
            [PROGRAM, opt],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True, preexec_fn=pin(core)
        )

    results = {}
    for opt, p in procs.items():
        out, err = p.communicate()
        if p.returncode != 0:
            print(f"option f'{opt} exited with f{p.returncode}:\n{err}", file=sys.stderr)
        results[opt] = parse_average(out, opt)

    n_iter = max((len(v) for v in results.values()), default=0)
    with open(OUT, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Test Size", "Linear", "Las Vegas", "Las Vegas (with bitmap)", "Parallel"])
        for i in range(n_iter):
            row = [100 * pow(10, i)]
            for opt in OPTIONS:
                v = results[opt]
                row.append(f"{v[i]:.6f}" if i < len(v) else "")
            w.writerow(row)

    print(f"{n_iter} test cases written to results.csv")

if __name__ == "__main__":
    main()