#!/bin/bash

# Set variables to collect measurement data
BINARY="./assignment1/main"
THREADS=(1 2 4 8 10)
RUNS=10
OUT="equal_chunks.csv"

echo "1,2,4,8,10" > "$OUT"

# Run the test up to 10 times and put the benchmark times into a separate CSV file
for ((i=1; i<=RUNS; i++))
do
	row=""

	for t in "${THREADS[@]}"
	do
		# Only extract real time
		/usr/bin/time -f "%e" -o tmp_time.txt "$BINARY" --jobs "$t"

		val=$(cat tmp_time.txt)
		row+="$val,"
	done

	row=${row%,}
	echo "$row" >> "$OUT"
done
