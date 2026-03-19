#include "mouse.h"

int set_led_colour(const mouse_dev_t *mouse, const struct led_packet led) {
    int ret;
    unsigned char *buf = kzalloc(20, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    // Step 1 - colour packet
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

    ret = usb_control_msg(mouse->usb_dev,
                          usb_sndctrlpipe(mouse->usb_dev, 0),
                          0x09, 0x21, 0x0211, 1,
                          buf, 20, USB_CTRL_SET_TIMEOUT);
    printk(KERN_INFO "set_led_colour: colour packet returned %d\n", ret);
    if (ret < 0) {
        kfree(buf);
        return ret;
    }

    msleep(10);

    // Step 2 - commit packet
    memset(buf, 0, 20);
    buf[0] = 0x11;
    buf[1] = 0xFF;
    buf[2] = 0x12;
    buf[3] = 0x70;

    ret = usb_control_msg(mouse->usb_dev,
                          usb_sndctrlpipe(mouse->usb_dev, 0),
                          0x09, 0x21, 0x0211, 1,
                          buf, 20, USB_CTRL_SET_TIMEOUT);
    printk(KERN_INFO "set_led_colour: commit returned %d\n", ret);
    kfree(buf);
    return (ret < 0) ? ret : 0;
}

int set_mouse_dpi(const mouse_dev_t *mouse, const int dpi) {
    unsigned char *buf = kzalloc(20, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    buf[0] = 0x11;
    buf[1] = 0xFF;
    buf[2] = 0x60;
    buf[3] = 0x1C;
    buf[4] = (dpi >> 8) & 0xFF;
    buf[5] = dpi & 0xFF;

    int ret = usb_control_msg(
        mouse->usb_dev,
        usb_sndctrlpipe(mouse->usb_dev, 0),
        0x09,
        USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        0x0211, 1, buf, 20, USB_CTRL_SET_TIMEOUT
    );
    kfree(buf);
    return (ret < 0) ? ret : 0;
}

long mouse_ioctl( struct file *file, const unsigned int cmd, const unsigned long arg) {
    int ret = 0;
    mouse_dev_t *mouse = file->private_data;
    switch (cmd) {
        case MOUSE_SET_LEDS: {
            struct led_packet p;
            if (copy_from_user(&p, (struct led_packet __user *) arg, sizeof(struct led_packet)))
                return -EFAULT;
            ret = set_led_colour(mouse, p);
            break;
        }
        case MOUSE_GET_LEDS:
            break;
        case MOUSE_SET_DPI: {
            int dpi_val;
            if (copy_from_user(&dpi_val, (int __user *) arg, sizeof(int)))
                return -EFAULT;
            if (dpi_val < 200 || dpi_val > 8000) return -EINVAL;
            ret = set_mouse_dpi(mouse, dpi_val);
            break;
        }
        case MOUSE_GET_DPI:
            break;
        default:
            return -ENOTTY;
    }
    return ret;
}
