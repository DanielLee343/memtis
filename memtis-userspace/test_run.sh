#!/bin/bash

LOGFILE="bench_run.log"

BENCH_PARAMS=("1:0" "1:1" "1:2" "1:4" "1:8" "1:16")
BENCH_PARAMS_no16=("1:0" "1:1" "1:2" "1:4" "1:8")

run_bench_1() {
    echo "$1 with memtis"
    ./scripts/run_bench.sh -B XSBench -V memtis -R "$1" --cxl
    sleep 5
    echo "$1 no memtis"
    ./scripts/run_bench.sh -B XSBench -V nomemtis -R "$1" --cxl --nomemtis
}
run_bench_2() {
    echo "$1 with memtis"
    ./scripts/run_bench.sh -B stream -V memtis -R "$1" --cxl
    sleep 5
    echo "$1 no memtis"
    ./scripts/run_bench.sh -B stream -V nomemtis -R "$1" --cxl --nomemtis
}

{
    echo "DRAM"
    numactl -N 0 -m 0 -- /mnt/newdrive/memtis/memtis-userspace/bench_dir/XSBench/openmp-threading/XSBench -t 24 -g 130000 -p 10000000
    sleep 10

    for param in "${BENCH_PARAMS[@]}"; do
        run_bench_1 "$param"
        sleep 10
    done

    echo "CXL"
    numactl -N 0 -m 1 -- /mnt/newdrive/memtis/memtis-userspace/bench_dir/XSBench/openmp-threading/XSBench -t 24 -g 130000 -p 10000000

    echo "----------------------- instru DRAM"
    numactl -N 0 -m 0 -- /mnt/newdrive/memtis/memtis-userspace/bench_dir/XSBench/openmp-threading/XSBench_instru -t 24 -g 130000 -p 10000000
    sleep 5
    echo "----------------------- instru 1:4 memtis"
    ./scripts/run_bench.sh -B XSBench -V instru_memtis -R 1:4 --cxl --instru
    sleep 5
    echo "----------------------- instru 1:4 nomemtis"
    ./scripts/run_bench.sh -B XSBench -V instru_nomemtis -R 1:4 --cxl --instru --nomemtis
    sleep 5
    echo "----------------------- instru CXL"
    numactl -N 0 -m 1 -- /mnt/newdrive/memtis/memtis-userspace/bench_dir/XSBench/openmp-threading/XSBench_instru -t 24 -g 130000 -p 10000000
} >> "$LOGFILE"

{
    echo "***********Next benchmark***********"
    echo "DRAM"
    numactl -N 0 -m 0 -- /mnt/newdrive/memtis/memtis-userspace/bench_dir/stream/stream
    sleep 10

    for param in "${BENCH_PARAMS_no16[@]}"; do
        run_bench_2 "$param"
        sleep 10
    done

    echo "CXL"
    numactl -N 0 -m 1 -- /mnt/newdrive/memtis/memtis-userspace/bench_dir/stream/stream

    echo "----------------------- instru DRAM"
    numactl -N 0 -m 0 -- /mnt/newdrive/memtis/memtis-userspace/bench_dir/stream/stream_instru
    sleep 5
    echo "----------------------- instru 1:4 memtis"
    ./scripts/run_bench.sh -B stream -V instru_memtis -R 1:4 --cxl --instru
    sleep 5
    echo "----------------------- instru 1:4 nomemtis"
    ./scripts/run_bench.sh -B stream -V instru_nomemtis -R 1:4 --cxl --instru --nomemtis
    sleep 5
    echo "----------------------- instru CXL"
    numactl -N 0 -m 1 -- /mnt/newdrive/memtis/memtis-userspace/bench_dir/stream/stream_instru
} >> "$LOGFILE"