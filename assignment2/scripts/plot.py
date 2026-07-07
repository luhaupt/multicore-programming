#!/usr/bin/env python3

import sys

import matplotlib.cm as cm
import matplotlib.pyplot as plt
import pandas as pd

files = sys.argv[1:]

# Combine all CSV files
df = pd.concat([pd.read_csv(filepath) for filepath in files], ignore_index=True)

# Split locks into normal locks and backoff locks
backoff_df = df[df["lock"].str.contains("backoff", case=False, na=False)]
normal_df = df[~df["lock"].str.contains("backoff", case=False, na=False)]

# Create two plots in the same image
fig, axes = plt.subplots(2, 1, figsize=(10, 12), sharex=True)


# Function to plot locks
def plot_locks(ax, data, title):
    locks = sorted(data["lock"].unique())

    # Generate unique colors
    color_map = {
        lock: cm.tab20(i / max(1, len(locks) - 1)) for i, lock in enumerate(locks)
    }

    for lock, lock_df in data.groupby("lock"):
        grouped = lock_df.groupby("jobs")["runtime_ns"]

        mean = grouped.mean()
        std = grouped.std()

        color = color_map[lock]

        # Deviation background
        ax.fill_between(
            mean.index,
            mean.values - std.values,
            mean.values + std.values,
            color=color,
            alpha=0.15,
        )

        # Runtime curve
        ax.errorbar(
            mean.index,
            mean.values,
            yerr=std.values,
            marker="o",
            capsize=4,
            label=lock,
            color=color,
        )

    ax.set_ylabel("Mean runtime (ns)")
    ax.set_title(title)
    ax.legend(title="Lock")
    ax.grid(True)


# Plot normal locks
plot_locks(axes[0], normal_df, "Lock performance comparison")

# Plot backoff locks
plot_locks(axes[1], backoff_df, "Lock performance comparison (backoff)")

axes[1].set_xlabel("Number of threads (jobs)")

plt.tight_layout()
plt.savefig("comparison.png", dpi=300)
plt.show()
