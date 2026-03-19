#ifndef MOUSE_H
#define MOUSE_H

#include <linux/cdev.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include "../include/shared_ioctl.h"
#include "mouse_features.h"


#define VENDOR_ID   0x046d
#define PRODUCT_ID  0xc092
#define BUFFER_SIZE 8


typedef struct mouse_dev_t {
    struct usb_device *usb_dev;
    struct usb_interface *intfs;
    struct urb *urb;
    struct input_dev *input_dev;
    dev_t dev;
    struct cdev cdev;
    unsigned char *buffer;
    bool read_ready;
    struct wait_queue_head read_queue;
    struct proc_dir_entry *proc_entry;
} mouse_dev_t;

// mouse_usb
extern struct usb_driver mouse_driver;

// mouse_char
int mouse_char_setup(void);
void mouse_char_teardown(void);
int mouse_char_init(mouse_dev_t *mouse);
void mouse_char_exit(mouse_dev_t *mouse);

int mouse_proc_init(mouse_dev_t *mouse);
void mouse_proc_exit(mouse_dev_t *mouse);

ssize_t mouse_read(struct file *f, char __user *user, size_t l, loff_t *o);

int set_led_colour(const mouse_dev_t *mouse, struct led_packet led);

int set_dpi(mouse_dev_t *mouse, int dpi);

long mouse_ioctl( struct file *file, unsigned int cmd, unsigned long arg);
#endif

