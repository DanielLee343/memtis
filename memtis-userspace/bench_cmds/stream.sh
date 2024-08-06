#!/bin/bash

BIN=/mnt/newdrive/memtis/memtis-userspace/bench_dir/stream
BENCH_RUN="${BIN}/stream"
BENCH_DRAM=""

# 45773 MB (44.7GB)
if [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="2693MB"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="5086MB"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="9155MB"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="15258MB"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="22887MB"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="75000MB"
fi


export BENCH_RUN
export BENCH_DRAM
