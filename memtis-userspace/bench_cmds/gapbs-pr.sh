#!/bin/bash

######## changes the below path
BIN=/mnt/newdrive/memtis/memtis-userspace/bench_dir/gapbs
GRAPH_DIR=/mnt/newdrive/memtis/memtis-userspace/bench_dir/gapbs/benchmark/graphs
if [[ -z "$1" ]]; then
    BENCH_RUN="${BIN}/pr -f ${GRAPH_DIR}/twitter.sg -i1000 -t1e-4 -n5"
else
    BENCH_RUN="${BIN}/pr_instru -f ${GRAPH_DIR}/twitter.sg -i1000 -t1e-4 -n5"
fi
# BENCH_RUN="${BIN}/pr -f ${GRAPH_DIR}/twitter.sg -i1000 -t1e-4 -n5"
BENCH_DRAM=""


# twitter.sg: ~12142MB
# anon: 13277MB

if [[ "x${NVM_RATIO}" == "x1:32" ]]; then
    BENCH_DRAM="753MB"
    elif [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="1463MB"
    elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="2762MB"
    elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="4972MB"
    elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="8286MB"
    elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="12429MB"
    elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="70000MB"
fi


export BENCH_RUN
export BENCH_DRAM
