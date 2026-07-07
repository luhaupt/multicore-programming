#!/usr/bin/env python3

import sys

import pandas as pd

files = sys.argv[1:]

# Load all CSV files
df = pd.concat([pd.read_csv(filepath) for filepath in files], ignore_index=True)

# Calculate mean runtime for every lock/job combination
means = df.groupby(["lock", "jobs"])["runtime_ns"].mean().reset_index()

locks = sorted(means["lock"].unique())
jobs = sorted(means["jobs"].unique())

results = []

for lock_a in locks:
    for lock_b in locks:
        # Skip comparing a lock with itself
        if lock_a == lock_b:
            continue

        for job in jobs:
            mean_a = means[(means["lock"] == lock_a) & (means["jobs"] == job)][
                "runtime_ns"
            ]

            mean_b = means[(means["lock"] == lock_b) & (means["jobs"] == job)][
                "runtime_ns"
            ]

            # Skip missing combinations
            if mean_a.empty or mean_b.empty:
                continue

            mean_a = mean_a.iloc[0]
            mean_b = mean_b.iloc[0]

            results.append(
                {
                    "lock_a": lock_a,
                    "!=": mean_a != mean_b,
                    "<": mean_a < mean_b,
                    ">": mean_a > mean_b,
                    "lock_b": lock_b,
                    "jobs": job,
                }
            )

result_df = pd.DataFrame(results)
result_df = result_df.sort_values(
    [
        "lock_a",
        "lock_b",
        "jobs",
    ]
)

print(result_df.to_csv(index=False))
