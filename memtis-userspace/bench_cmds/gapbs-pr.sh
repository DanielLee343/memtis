#!/bin/bash

######## changes the below path
BIN=/mnt/newdrive/memtis/memtis-userspace/bench_dir/gapbs
GRAPH_DIR=/mnt/newdrive/memtis/memtis-userspace/bench_dir/gapbs/benchmark/graphs

BENCH_RUN="${BIN}/pr -f ${GRAPH_DIR}/twitter.sg -i1000 -t1e-4 -n5"
BENCH_DRAM=""


# twitter.sg: ~12142MB
# anon: 13277MB

if [[ "x${NVM_RATIO}" == "x1:32" ]]; then
    BENCH_DRAM="12545MB"
elif [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="12923MB"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="13618MB"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="14798MB"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="16568MB"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="18781MB"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="70000MB"
fi


export BENCH_RUN
export BENCH_DRAM
