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
#include <linux/mutex.h>
#include "../include/shared_ioctl.h"


#define VENDOR_ID   0x046d
#define PRODUCT_ID  0xc092

#define BUFFER_SIZE 8


typedef struct mouse_dev_t {
    //usb stuff
    struct usb_device *usb_dev;
    struct usb_interface *intfs;
    struct urb *urb;
    // input
    struct input_dev *input_dev;
    //char device
    dev_t dev;
    struct cdev cdev;
    //blocking read
    struct wait_queue_head read_queue;
    bool read_ready;
    //lock to copy to read
    spinlock_t read_lock;
    //ioctl lock
    struct mutex ioctl_lock;
    // proc enrty
    struct proc_dir_entry *proc_entry;
    //buffer ptr
    unsigned char *buffer;
} mouse_dev_t;

// mouse_usb
extern struct usb_driver mouse_driver;

// mouse_char
int mouse_char_setup(void);

void mouse_char_teardown(void);

int mouse_char_init(mouse_dev_t *mouse);

void mouse_char_exit(mouse_dev_t *mouse);

ssize_t mouse_read(struct file *f, char __user *user_buffer, size_t l, loff_t *o);

//mouse proc
int mouse_proc_init(mouse_dev_t *mouse);

void mouse_proc_exit(mouse_dev_t *mouse);


int set_led_colour(const mouse_dev_t *mouse, struct led_packet led);

int set_dpi(mouse_dev_t *mouse, int dpi);

long mouse_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif

