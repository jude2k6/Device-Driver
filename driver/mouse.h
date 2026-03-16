#ifndef MOUSE_H
#define MOUSE_H

#include <linux/cdev.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/spinlock.h>


#define VENDOR_ID   0x046d
#define PRODUCT_ID  0xc092
#define BUFFER_SIZE 8


typedef struct mouse_dev_t {
    struct usb_device *usb_dev;
    struct usb_interface *intfs;
    struct input_dev *input_dev;
    struct urb *urb;
    unsigned char *buffer;
    bool read_ready;
    struct wait_queue_head read_queue;

    spinlock_t lock;
    bool connected;
    u64 packets_seen;
    u8 buttons_raw;
    s8 last_wheel;
} mouse_dev_t;

// mouse_usb
extern struct usb_driver mouse_driver;
extern mouse_dev_t *mouse;

// mouse_char
int mouse_char_init(void);

void mouse_char_exit(void);

ssize_t mouse_read(struct file *f, char __user *user, size_t l, loff_t *o);

int set_led_colour(unsigned char r, unsigned char g, unsigned char b, unsigned char mode);

int set_dpi(void);

int mouse_proc_init(void);

void mouse_proc_exit(void);
#endif

