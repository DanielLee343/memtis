#!/bin/bash

TARGET=$1
CGROUP_NAME=$2
MEM_FP_FILE=fp_raw.csv

while :
do
    cat /sys/fs/cgroup/${CGROUP_NAME}/memory.stat | grep -e anon_thp -e anon >> ${TARGET}/memory_stat.txt
    cat /sys/fs/cgroup/${CGROUP_NAME}/memory.hotness_stat >> ${TARGET}/hotness_stat.txt
    cat /proc/vmstat | grep pgmigrate_su >> ${TARGET}/pgmig.txt
    numactl -H | grep free >>${TARGET}/${MEM_FP_FILE}
    sleep 1
done
