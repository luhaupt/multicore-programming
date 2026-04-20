import pandas as pd
import matplotlib.pyplot as plt

def analyze(file):
	df = pd.read_csv(file)

	means = df.mean()
	stds = df.std()

	return means, stds

m1, s1 = analyze("approach1.csv")
m2, s2 = analyze("approach2.csv")

threads = [1, 2, 4, 8, 10]

plt.errorbar(threads, m1, yerr=s1, label="Approach 1", marker='o')
plt.errorbar(threads, m2, yerr=s2, label="Approach 2", marker='o')

plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (s)")
plt.legend()
plt.grid(True)
plt.show()
