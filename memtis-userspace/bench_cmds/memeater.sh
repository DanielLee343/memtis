#!/bin/bash

######## changes the below path
BIN=/mnt/newdrive/memtis/memtis-userspace/bench_dir/memeater
# GRAPH_DIR=/mnt/newdrive/memtis/memtis-userspace/bench_dir/gapbs/benchmark/graphs

BENCH_RUN="${BIN}/memeater"
BENCH_DRAM=""

# memeater: 16GB (16384MB)

if [[ "x${NVM_RATIO}" == "x1:32" ]]; then
    BENCH_DRAM="497MB"
elif [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="964MB"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="1821MB"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="3277MB"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="5462MB"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="8192MB"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="20000MB"
fi


export BENCH_RUN
export BENCH_DRAM
