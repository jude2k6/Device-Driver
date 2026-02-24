#!/bin/bash
sudo insmod ../driver/my_driver.ko
sudo mknod /dev/mydevice c $(grep my_driver /proc/devices | awk '{print $1}') 0
dmesg | tail
