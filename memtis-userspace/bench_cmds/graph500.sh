#!/bin/bash

BIN=/mnt/newdrive/memtis/memtis-userspace/bench_dir/graph500/omp-csr
BENCH_RUN="${BIN}/omp-csr -s 24 -e 15"
BENCH_DRAM=""
# -s 24: 8453 MB

if [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="498MB"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="940MB"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="1691MB"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="2818MB"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="4227MB"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="75000MB"
fi
# if [[ "x${NVM_RATIO}" == "x1:16" ]]; then
#     BENCH_DRAM="3850MB"
# elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
#     BENCH_DRAM="7200MB"
# elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
#     BENCH_DRAM="13107MB"
# elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
#     BENCH_DRAM="21800MB"
# elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
#     BENCH_DRAM="32768MB"
# elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
#     BENCH_DRAM="75000MB"
# fi


export BENCH_RUN
export BENCH_DRAM
