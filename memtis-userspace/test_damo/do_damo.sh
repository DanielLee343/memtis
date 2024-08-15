/mnt/newdrive/memtis/memtis-userspace/bench_dir/gapbs/pr -g 27 -t1e-4 -n 10 >out.txt 2>&1 &
check_pid=$!
sudo damo record -o /mnt/newdrive/memtis/memtis-userspace/test_damo/damo.data $check_pid
