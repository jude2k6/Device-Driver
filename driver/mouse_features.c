
#include "mouse.h"



int set_led_colour(mouse_dev_t *mouse, struct led_packet led) {
    // Lightsync uses a 20-byte HID++ 2.0 packet
    // Buffer MUST be kmalloc'd for USB DMA — stack buffers cause EAGAIN
    int ret;
    unsigned char *buf = kzalloc(20, GFP_KERNEL);
    if (!buf) {
        return -ENOMEM;
    }


    buf[0] = 0x11;
    buf[1] = 0xFF;
    buf[2] = 0x12;
    buf[3] = 0x10;
    buf[4] = 0x01;
    buf[5] = led.r;
    buf[6] = led.g;
    buf[7] = led.b;
    buf[8] = 0x02;
    buf[9] = led.r;
    buf[10] = led.g;
    buf[11] = led.b;
    buf[12] = 0x03;
    buf[13] = led.r;
    buf[14] = led.g;
    buf[15] = led.b;
    buf[16] = 0xFF;
    printk(KERN_INFO "set_led_colour: sending colour packet\n");
    printk(
        KERN_INFO
        "buf: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
        buf[0], buf[1], buf[2], buf[3], buf[4],
        buf[5], buf[6], buf[7], buf[8], buf[9],
        buf[10], buf[11], buf[12], buf[13], buf[14],
        buf[15], buf[16], buf[17], buf[18], buf[19]);
    ret = usb_control_msg(mouse->usb_dev,
                          usb_sndctrlpipe(mouse->usb_dev, 0),
                          0x09, 0x21, 0x0211, 1,
                          buf, 20, USB_CTRL_SET_TIMEOUT);
    printk(KERN_INFO "set_led_colour: colour packet returned %d\n", ret);
    if (ret < 0) {
        printk(KERN_ERR "set_led_colour: colour packet failed with %d\n", ret);
        kfree(buf);
        return ret;
    }

    msleep(10);
    // Step 2 - commit packet (applies colour to RAM)
    memset(buf, 0, 20);
    buf[0] = 0x11;
    buf[1] = 0xFF;
    buf[2] = 0x12;
    buf[3] = 0x70;

    ret = usb_control_msg(mouse->usb_dev,
                          usb_sndctrlpipe(mouse->usb_dev, 0),
                          0x09, 0x21, 0x0211, 1,
                          buf, 20, USB_CTRL_SET_TIMEOUT);
    if (ret < 0) {
        printk(KERN_ERR "set_led_colour: commit packet failed with %d\n", ret);
        mouse->transaction_open = true;
        kfree(buf);
        return ret;
    }

    msleep(10);
    // Step 3 - persist to onboard memory (survives the ~30s RAM timeout)
    // Feature 0x1300 (onboard profiles), function 0x10 = save current profile
    memset(buf, 0, 20);
    buf[0] = 0x11;
    buf[1] = 0xFF;
    buf[2] = 0x13;  // feature index for onboard profiles (0x1300)
    buf[3] = 0x10;  // saveCurrentProfile function

    ret = usb_control_msg(mouse->usb_dev,
                          usb_sndctrlpipe(mouse->usb_dev, 0),
                          0x09, 0x21, 0x0211, 1,
                          buf, 20, USB_CTRL_SET_TIMEOUT);
    printk(KERN_INFO "set_led_colour: persist packet returned %d\n", ret);
    mouse->transaction_open = false;
    kfree(buf);
    return (ret < 0) ? ret : 0;
}

int set_mouse_dpi(mouse_dev_t *mouse, int dpi) {
    // Buffer MUST be kmalloc'd for USB DMA — stack buffers cause EAGAIN
    unsigned char *buf = kzalloc(20, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    buf[0] = 0x11;
    buf[1] = 0xFF;
    buf[2] = 0x60; //dpi
    buf[3] = 0x1C;
    buf[4] = (dpi >> 8) & 0xFF;
    buf[5] = dpi & 0xFF;

    int ret = usb_control_msg(
        mouse->usb_dev,
        usb_sndctrlpipe(mouse->usb_dev, 0),
        0x09,
        USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        0x0211,
        1,
        buf,
        20,
        USB_CTRL_SET_TIMEOUT
    );
    kfree(buf);
    return (ret < 0) ? ret : 0;
}


long mouse_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    mouse_dev_t *mouse = file->private_data;
    switch (cmd) {
        case MOUSE_SET_LEDS: {
            printk(KERN_ERR"entered colour ioctl");
            struct led_packet p;
            if (copy_from_user(&p, (struct led_packet __user *) arg, sizeof(struct led_packet))) {
                return -EFAULT;
            }
            ret = set_led_colour(mouse, p);
            break;
        }
        case MOUSE_GET_LEDS: {
            break;
        }
        case MOUSE_SET_DPI: {
            int dpi_val;
            if (copy_from_user(&dpi_val, (int __user *) arg, sizeof(int))) {
                return -EFAULT;
            }
            // G203 range is usually 200 - 8000
            if (dpi_val < 200 || dpi_val > 8000) { return -EINVAL; }
            ret = set_mouse_dpi(mouse, dpi_val);

            break;
        }
        case MOUSE_GET_DPI: {
            break;
        }
        default:
            return -ENOTTY;
    }
    return ret;
}

