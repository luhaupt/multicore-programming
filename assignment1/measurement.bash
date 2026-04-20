#!/bin/bash

BINARY="./assignment1/main"
THREADS=(1 2 4 8 10)
RUNS=10
OUT="equal_chunks.csv"

echo "1,2,4,8,10" > "$OUT"

for ((i=1; i<=RUNS; i++))
do
	row=""

	for t in "${THREADS[@]}"
	do
		# time nur real Zeit extrahieren
		/usr/bin/time -f "%e" -o tmp_time.txt "$BINARY" --jobs "$t"

		val=$(cat tmp_time.txt)
		row+="$val,"
	done

	row=${row%,}
	echo "$row" >> "$OUT"
done
