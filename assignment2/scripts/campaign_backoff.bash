#!/bin/bash

PAIRS=(
	"0 100"
	"25 100"
	"50 200"
	"50 400"
	"100 400"
	"100 800"
	"200 800"
	"200 1600"
	"500 2000"
	"500 4000"
	"1000 8000"
)

JOBS=(1 2 4 6 8 10)

SAMPLES=50


for pair in "${PAIRS[@]}"
do
	read MIN MAX <<< "$pair"

	FILE="results/backoff-${MIN}-${MAX}.csv"

	echo "jobs,lock,runtime_ns" > "$FILE"

	for jobs in "${JOBS[@]}"
	do
		./a.out \
			--lock backoff \
			--backoff-min "$MIN" \
			--backoff-max "$MAX" \
			--jobs "$jobs" \
			--samples "$SAMPLES" \
			>> "$FILE"
	done
done
