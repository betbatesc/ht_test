#!/usr/bin/env bash

# This script runs a Hash Join test and gets profile data using "perf" tool.

work=.
sf=1

echo "Running Hash Join test along perf report tool"
./main -work $work -s $sf &
app_pid=$!
echo "App PID: $app_pid"
kill -0 $app_pid
perf stat -v -d -a --pid $app_pid -e branches,branch-misses,cache-misses,cache-references,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,LLC-loads,LLC-load-misses,instructions,cycles &
perf_pid=$!
kill -0 $perf_pid
echo "Perf PID: $perf_pid"
echo "Wait for perf record"
wait $perf_pid
echo "Perf record done"

