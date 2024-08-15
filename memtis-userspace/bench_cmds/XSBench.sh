#!/bin/bash

BIN=/mnt/newdrive/memtis/memtis-userspace/bench_dir/XSBench/openmp-threading
if [[ -z "$1" ]]; then
  BENCH_RUN="${BIN}/XSBench -t 24 -g 130000 -p 10000000"
else
  BENCH_RUN="${BIN}/XSBench_instru -t 24 -g 130000 -p 10000000"
fi
# BENCH_RUN="${BIN}/XSBench -t 24 -g 130000 -p 10000000"
BENCH_DRAM=""

# -g 65000: 32625 MB
# -g 130000: 65219 MB
# if [[ "x${NVM_RATIO}" == "x1:32" ]]; then
#   BENCH_DRAM="990MB"
# elif [[ "x${NVM_RATIO}" == "x1:16" ]]; then
#   BENCH_DRAM="1920MB"
# elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
#   BENCH_DRAM="3625MB"
# elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
#   BENCH_DRAM="6525MB"
# elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
#   BENCH_DRAM="10875MB"
# elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
#   BENCH_DRAM="16313MB"
# elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
#   BENCH_DRAM="75000MB"
# fi
if [[ "x${NVM_RATIO}" == "x1:16" ]]; then
  BENCH_DRAM="3850MB"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
  BENCH_DRAM="7200MB"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
  BENCH_DRAM="13107MB"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
  BENCH_DRAM="21800MB"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
  BENCH_DRAM="32768MB"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
  BENCH_DRAM="75000MB"
fi

export BENCH_RUN
export BENCH_DRAM
