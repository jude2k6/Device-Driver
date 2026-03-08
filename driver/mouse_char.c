#include "mouse.h"

static dev_t dev;
static struct cdev cdev;
static struct class *mouse_class;

ssize_t mouse_read(struct file *f, char __user *user, size_t l, loff_t *o) {
    printk("Read Called");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = mouse_read
};


int mouse_char_init(void) {
    if (alloc_chrdev_region(&dev, 0, 1, "MOUSE")) {
        printk(KERN_ERR "ERROR alloc_chrdev_region");
        goto alloc_fail;
    }
    cdev_init(&cdev, &fops);
    if (cdev_add(&cdev, dev, 1)) {
        printk(KERN_ERR "ERROR cdev_add");
        goto cdev_fail;
    }

    mouse_class = class_create("MOUSE");
    if (IS_ERR(mouse_class)) {
        printk(KERN_ERR "ERROR class_create");
        goto class_fail;
    }

    if (IS_ERR(device_create(mouse_class, NULL, dev, NULL, "MOUSE"))) {
        printk(KERN_ERR "ERROR device_create");
        goto device_fail;
    }

    printk(KERN_INFO "Mouse char init SUCCESS\n");
    return 0;

device_fail:
    class_destroy(mouse_class);
class_fail:
    cdev_del(&cdev);
cdev_fail:
    unregister_chrdev_region(dev, 1);
alloc_fail:
    return -1;
}

void mouse_char_exit(void) {
    cdev_del(&cdev);
    device_destroy(mouse_class, dev);
    class_destroy(mouse_class);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Mouse char cleaned\n");
}
