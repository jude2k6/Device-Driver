#include "mouse.h"
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

#define PROC_NAME "MOUSE"

static struct proc_dir_entry *mouse_proc_entry;

static ssize_t mouse_proc_read(struct file *file, char __user *user_buf, size_t count, loff_t *pos) {
    mouse_dev_t *mouse = (mouse_dev_t *) file->private_data;


    char buf[512];
    int len;
    unsigned long flags;

    bool urb_allocated;
    unsigned char sample[8] = {0};
    dev_t devnum;
    const char *input_name = NULL;
    int interface_num = 0;
    u16 vendor = 0, product = 0;
    u8 bus = 0, device = 0;


    urb_allocated = mouse->urb != NULL;
    devnum = mouse->dev;
    input_name = mouse->input_dev->name;
    vendor = mouse->usb_dev->descriptor.idVendor;
    product = mouse->usb_dev->descriptor.idProduct;
    bus = mouse->usb_dev->bus->busnum;
    device = mouse->usb_dev->devnum;
    interface_num = mouse->intfs->cur_altsetting->desc.bInterfaceNumber;

    memcpy(sample, mouse->buffer, sizeof(sample));


    len = scnprintf(buf, sizeof(buf),
                    "Mouse Device Info:\n"
                    "USB Vendor/Product: %04x/%04x\n"
                    "Bus/Device: %u/%u\n"
                    "Interface: %d\n"
                    "Character device: %u:%u\n"
                    "Input device: %s\n"
                    "URB allocated: %s\n"
                    "Buffer sample: %02x %02x %02x %02x %02x %02x %02x %02x ...\n",
                    vendor, product,
                    bus, device,
                    interface_num,
                    MAJOR(devnum), MINOR(devnum),
                    input_name,
                    urb_allocated ? "true" : "false",
                    sample[0], sample[1], sample[2], sample[3],
                    sample[4], sample[5], sample[6], sample[7]
    );

    return simple_read_from_buffer(user_buf, count, pos, buf, len);
}

static int mouse_proc_open(struct inode *inode, struct file *file) {
    file->private_data = pde_data(inode);
    return 0;
}

static const struct proc_ops mouse_proc_ops = {

    .proc_read = mouse_proc_read,
    .proc_open = mouse_proc_open,

};

int mouse_proc_init(mouse_dev_t *mouse) {
    mouse->proc_entry=  proc_create_data(PROC_NAME, 0444, NULL, &mouse_proc_ops, mouse);
    if (!mouse->proc_entry) {
        printk(KERN_ERR "Failed to create proc");
        return -ENOMEM;
    }
    printk(KERN_INFO "proc made");
    return 0;
}

void mouse_proc_exit(mouse_dev_t *mouse) {
    if (mouse->proc_entry) {
        proc_remove(mouse->proc_entry);
        mouse->proc_entry = NULL;
    }
    printk(KERN_INFO "proc removed");
}
