#!/bin/bash

# Must match IFACE in load.sh
IFACE="3-5:1.1"

sudo rmmod mouse

# Hand the interface back to the native usbhid driver
echo "$IFACE" | sudo tee /sys/bus/usb/drivers/usbhid/bind

sudo dmesg | tail -5
