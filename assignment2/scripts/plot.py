#!/usr/bin/env python3

import sys

import matplotlib.pyplot as plt
import pandas as pd

for filepath in sys.argv[1:]:
    df = pd.read_csv(filepath)

    grouped = df.groupby("jobs")["runtime_ns"]

    mean = grouped.mean()
    std = grouped.std()

    plt.errorbar(
        mean.index, mean.values, yerr=std.values, marker="o", capsize=4, label=filepath
    )


plt.xlabel("Number of jobs")
plt.ylabel("Average runtime (ns)")
plt.legend()
plt.grid(True)

plt.show()
