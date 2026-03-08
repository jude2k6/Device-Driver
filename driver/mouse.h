#ifndef MOUSE_H
#define MOUSE_H

#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define VENDOR_ID   0x046d
#define PRODUCT_ID  0xc092
#define BUFFER_SIZE 8

typedef struct mouse_dev_t {
    struct usb_device *dev;
    struct usb_interface *intfs;
    struct urb *urb;
    unsigned char *buffer;
    u8 dpi;
} mouse_dev_t;

// mouse_usb
extern struct usb_driver mouse_driver;

// mouse_char
int mouse_char_init(void);
void mouse_char_exit(void);
ssize_t mouse_read(struct file *f, char __user *user, size_t l, loff_t *o);

#endif