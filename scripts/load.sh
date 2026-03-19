#!/bin/bash

sudo insmod ../driver/mouse.ko
sudo dmesg | tail -5