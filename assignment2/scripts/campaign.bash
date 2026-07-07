#!/bin/bash

LOCKS=("atomic" "tas" "ttas" "aq")
JOBS=(1 2 4 6 8 10)
SAMPLES=50

for lock in "${LOCKS[@]}"
do
	output="../results/${lock}.csv"

	# CSV header
	echo "jobs,lock,runtime_ns" > "$output"

	for jobs in "${JOBS[@]}"
	do
		../a.out \
			--lock "$lock" \
			--jobs "$jobs" \
			--samples "$SAMPLES" \
			>> "$output"
	done
done
