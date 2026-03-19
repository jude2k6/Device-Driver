//
// Created by jude on 13/03/2026.
//

#ifndef DEVICE_DRIVER_SHARED_IOCTL_H
#define DEVICE_DRIVER_SHARED_IOCTL_H

#include <linux/ioctl.h>

#define LED_WRITE 1
#define LED_READ  2
#define DPI_WRITE 3
#define DPI_READ  4

#define DEV_MAGIC 42

struct led_packet {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

#define MOUSE_SET_LEDS  _IOW(DEV_MAGIC, LED_WRITE, struct led_packet)
#define MOUSE_GET_LEDS  _IOR(DEV_MAGIC, LED_READ, int)

#endif //DEVICE_DRIVER_SHARED_IOCTL_H