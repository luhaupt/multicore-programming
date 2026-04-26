import os

import matplotlib.pyplot as plt
import pandas as pd

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Calculate mean and standard deviation of each column
def analyze(file):
    df = pd.read_csv(file)

    means = df.mean()
    stds = df.std()

    return means, stds

# Analyze both CSV files
m1, s1 = analyze(os.path.join(ROOT, "equal_chunks.csv"))
m2, s2 = analyze(os.path.join(ROOT, "shared_counter.csv"))

# Create graph
threads = [1, 2, 4, 8, 10]

plt.errorbar(threads, m1, yerr=s1, label="Equal chunks", marker="o", capsize=5)
plt.errorbar(threads, m2, yerr=s2, label="Shared counter", marker="o", capsize=5)

plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (s)")
plt.legend()
plt.grid(True)
plt.savefig("assignment1/plot.png")
