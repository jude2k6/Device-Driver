



#include "mouse.h"

static dev_t dev;
static struct class *mouse_class;
static mouse_dev_t *mouse_ref;

ssize_t mouse_read(struct file *f, char __user *user_buffer, size_t l, loff_t *o) {
    mouse_dev_t *mouse = (mouse_dev_t *)f->private_data;
    mouse->read_ready = true;
    if (!(mouse->read_ready)) {
        wait_event_interruptible(mouse->read_queue, mouse->read_ready);
    }


    printk("Data sent | %02x %02x %02x %02x %02x %02x %02x %02x",

           mouse->buffer[0], mouse->buffer[1], mouse->buffer[2], mouse->buffer[3],
           mouse->buffer[4], mouse->buffer[5], mouse->buffer[6], mouse->buffer[7]);


    copy_to_user(user_buffer, mouse->buffer,BUFFER_SIZE);
    printk("Read Called");
    return BUFFER_SIZE;
}
static int mouse_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "Mouse char opened\n");
    mouse_dev_t *dev;

    dev = container_of(inode->i_cdev, mouse_dev_t, cdev);

    if (!dev)
        return -ENODEV;

    file->private_data = dev;

    return 0;
}
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mouse_open,
    .read = mouse_read
    ,.unlocked_ioctl = mouse_ioctl
};


int mouse_char_init(mouse_dev_t* mouse) {
    mouse_ref = mouse;
    if (alloc_chrdev_region(&dev, 0, 1, "MOUSE")) {
        printk(KERN_ERR "ERROR alloc_chrdev_region");
        goto alloc_fail;
    }
    cdev_init(&mouse->cdev, &fops);
    if (cdev_add(&mouse->cdev, dev, 1)) {
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
    cdev_del(&mouse->cdev);
cdev_fail:
    unregister_chrdev_region(dev, 1);
alloc_fail:
    return -1;
}

void mouse_char_exit(void) {
    if (mouse_ref)
        cdev_del(&mouse_ref->cdev);
    device_destroy(mouse_class, dev);
    class_destroy(mouse_class);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Mouse char cleaned\n");
}
