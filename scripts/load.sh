#!/bin/bash

# Set IFACE to the sysfs path of the mouse's data interface.
# To get it, run "sudo cat /sys/kernel/debug/usb/usbmon/(bus number from lsusb)u"
# and check the endpoint field, e.g. "Ii:3:007:2" = EP2 = interface 1. Update mouse_usb.c to match.
IFACE="3-5:1.1"

# Unbind from the default usbhid driver so our driver can take over
if [ -e "/sys/bus/usb/drivers/usbhid/$IFACE" ]; then
    echo "$IFACE" | sudo tee /sys/bus/usb/drivers/usbhid/unbind
fi

sudo insmod ../driver/mouse.ko

echo "$IFACE" | sudo tee /sys/bus/usb/drivers/MOUSE/bind

sudo dmesg | tail -5
