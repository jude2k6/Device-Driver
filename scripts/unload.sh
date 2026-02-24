#!/bin/bash
sudo rmmod my_driver
sudo rm -f /dev/mydevice
dmesg | tail
