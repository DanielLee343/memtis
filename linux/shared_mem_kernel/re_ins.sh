#!/bin/bash
make
sudo rmmod -f shared_mem_kernel.ko
sudo insmod shared_mem_kernel.ko
sudo chmod 666 /dev/shared_mem_device