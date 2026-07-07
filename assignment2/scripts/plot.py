#!/usr/bin/env python3

import sys

import matplotlib.cm as cm
import matplotlib.pyplot as plt
import pandas as pd

files = sys.argv[1:]

# Combine all CSV files
df = pd.concat([pd.read_csv(filepath) for filepath in files], ignore_index=True)

# Create two plots in the same image
fig, ax = plt.subplots(figsize=(10, 6))


# Function to plot locks
def plot_locks(ax, data, title):
    locks = sorted(data["lock"].unique())

    # Generate unique colors
    color_map = {
        lock: cm.tab20(i / max(1, len(locks) - 1)) for i, lock in enumerate(locks)
    }

    for lock, df in data.groupby("lock"):
        grouped = df.groupby("jobs")["runtime_ns"]

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
plot_locks(ax, df, "Lock performance comparison")

ax.set_xlabel("Number of threads (jobs)")


def print_lock_order(data, percentage=0.05):
    print("\nLock performance ordering (adaptive epsilon):")

    for jobs in sorted(data["jobs"].unique()):
        jobs_df = data[data["jobs"] == jobs]

        stats = (
            jobs_df.groupby("lock")["runtime_ns"]
            .agg(["mean", "std"])
            .fillna(0)
            .sort_values("mean")
        )

        locks = list(stats.index)

        groups = []
        current_group = [locks[0]]

        for i in range(1, len(locks)):
            lock_a = locks[i - 1]
            lock_b = locks[i]

            mean_a = stats.loc[lock_a, "mean"]
            mean_b = stats.loc[lock_b, "mean"]

            std_a = stats.loc[lock_a, "std"]
            std_b = stats.loc[lock_b, "std"]

            # Adaptive epsilon
            epsilon = max(percentage * min(mean_a, mean_b), std_a + std_b)

            difference = abs(mean_b - mean_a)

            if difference <= epsilon:
                current_group.append(lock_b)
            else:
                groups.append(current_group)
                current_group = [lock_b]

        groups.append(current_group)

        order = " <= ".join(
            ",".join(group) if len(group) > 1 else group[0] for group in groups
        )

        print(f"jobs={jobs}: {order}")


print_lock_order(df)


plt.tight_layout()
plt.savefig("comparison.png", dpi=300)
plt.show()
