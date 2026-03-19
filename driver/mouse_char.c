#include "mouse.h"
#define MAX_MICE 8

static struct class *mouse_class;
static dev_t mouse_dev_base;


ssize_t mouse_read(struct file *f, char __user *user, size_t l, loff_t *o) {
    mouse_dev_t *mouse = f->private_data;
    if (!mouse->read_ready) {
        wait_event_interruptible(mouse->read_queue, mouse->read_ready);
    }
    mouse->read_ready = false; 

    printk("Data sent | %02x %02x %02x %02x %02x %02x %02x %02x",

           mouse->buffer[0], mouse->buffer[1], mouse->buffer[2], mouse->buffer[3],
           mouse->buffer[4], mouse->buffer[5], mouse->buffer[6], mouse->buffer[7]);

    // return EFAULT if copying to userspace fails
    if (copy_to_user(user_buffer, mouse->buffer, BUFFER_SIZE))
        return -EFAULT;

    copy_to_user(user, mouse->buffer,BUFFER_SIZE);
    printk("Read Called");
    return BUFFER_SIZE;
}

static int mouse_open(struct inode *inode, struct file *file) {
    printk(KERN_ALERT "Mouse char opened\n");
    mouse_dev_t *dev;

    dev = container_of(inode->i_cdev, mouse_dev_t, cdev);

    if (!dev) {
        return -ENODEV;
    }


    file->private_data = dev;

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mouse_open,
    .read = mouse_read,
    .unlocked_ioctl = mouse_ioctl
};


int mouse_char_setup(void) {
    if (alloc_chrdev_region(&mouse_dev_base, 0, MAX_MICE, "MOUSE")) {
        printk(KERN_ERR "ERROR alloc_chrdev_region");
        return -ENOMEM;
    }
    mouse_class = class_create("MOUSE");
    if (IS_ERR(mouse_class)) {
        printk(KERN_ERR "ERROR class_create");
        unregister_chrdev_region(mouse_dev_base, MAX_MICE);
        return PTR_ERR(mouse_class);
    }
    return 0;
}

void mouse_char_teardown(void) {
    class_destroy(mouse_class);
    unregister_chrdev_region(mouse_dev_base, MAX_MICE);
}

int mouse_char_init(mouse_dev_t *mouse) {
    mouse->dev = MKDEV(MAJOR(mouse_dev_base), 0);
    cdev_init(&mouse->cdev, &fops);
    if (cdev_add(&mouse->cdev, mouse->dev, 1)) {
        return -ENOMEM;
    }
    if (IS_ERR(device_create(mouse_class, NULL, mouse->dev, NULL, "MOUSE"))) {
        printk(KERN_ERR "ERROR device_create");
        return -ENOMEM;
    }
    printk(KERN_INFO "Mouse char init SUCCESS\n");
    return 0;
}

void mouse_char_exit(mouse_dev_t *mouse) {
    device_destroy(mouse_class, mouse->dev);
    cdev_del(&mouse->cdev);
}
