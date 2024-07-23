#!/bin/bash
# make
sudo rmmod -f poll_device.ko
sudo insmod poll_device.ko
sudo chmod 666 /dev/poll_device