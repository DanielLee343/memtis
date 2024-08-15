#!/bin/bash

######## changes the below path
BIN=/mnt/newdrive/memtis/memtis-userspace/bench_dir/gapbs
GRAPH_DIR=/mnt/newdrive/memtis/memtis-userspace/bench_dir/gapbs/benchmark/graphs
if [[ -z "$1" ]]; then
  #    BENCH_RUN="${BIN}/pr -f ${GRAPH_DIR}/twitter.sg -i1000 -t1e-4 -n5"
  BENCH_RUN="${BIN}/pr -g 27 -t1e-4 -n3"
else
  #    BENCH_RUN="${BIN}/pr_instru -f ${GRAPH_DIR}/twitter.sg -i1000 -t1e-4 -n5"
  BENCH_RUN="${BIN}/pr_instru -g 27 -t1e-4 -n3"
fi
# BENCH_RUN="${BIN}/pr -f ${GRAPH_DIR}/twitter.sg -i1000 -t1e-4 -n5"
BENCH_DRAM=""

# twitter.sg: ~12142MB
# anon: 13277MB

if [[ "x${NVM_RATIO}" == "x1:32" ]]; then
  BENCH_DRAM="1078MB"
elif [[ "x${NVM_RATIO}" == "x1:16" ]]; then
  BENCH_DRAM="2093MB"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
  BENCH_DRAM="3952MB"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
  BENCH_DRAM="7114MB"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
  BENCH_DRAM="11856MB"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
  BENCH_DRAM="17784MB"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
  BENCH_DRAM="70000MB"
fi

export BENCH_RUN
export BENCH_DRAM
