#!/bin/bash
make
sudo rmmod -f shared_addr_dev.ko
sudo insmod shared_addr_dev.ko
sudo chmod 666 /dev/shared_addr_dev