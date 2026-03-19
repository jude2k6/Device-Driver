#include "mouse.h"

int set_led_colour(const mouse_dev_t *mouse, const struct led_packet led) {
    int ret;
    unsigned char *buf = kzalloc(20, GFP_KERNEL);
    unsigned char *rbuf = kzalloc(20, GFP_KERNEL);
    if (!buf || !rbuf) {
        kfree(buf);
        kfree(rbuf);
        return -ENOMEM;
    }

    buf[0] = 0x10;
    buf[1] = 0xFF;
    buf[2] = 0x0E;
    buf[3] = 0x5B;
    buf[4] = 0x01;
    buf[5] = 0x03;
    buf[6] = 0x05;
    ret = usb_control_msg(mouse->usb_dev,
                          usb_sndctrlpipe(mouse->usb_dev, 0),
                          0x09, 0x21, 0x0210, 1,
                          buf, 20, USB_CTRL_SET_TIMEOUT);
    if (ret < 0) goto done;

    ret = usb_bulk_msg(mouse->usb_dev,
                       usb_rcvbulkpipe(mouse->usb_dev, 0x82),
                       rbuf, 20, NULL, 1000);
    msleep(10);

    memset(buf, 0, 20);
    buf[0] = 0x11;
    buf[1] = 0xFF;
    buf[2] = 0x0E;
    buf[3] = 0x1B;
    buf[4] = 0x00;
    buf[5] = 0x01;
    buf[6] = led.r;
    buf[7] = led.g;
    buf[8] = led.b;
    buf[9] = 0x00;
    buf[10] = 0x00;
    buf[11] = 0x00;
    buf[12] = 0x00;
    buf[13] = 0x00;
    buf[14] = 0x00;
    buf[15] = 0x00;
    buf[16] = 0x01;
    buf[17] = 0x00;
    buf[18] = 0x00;
    buf[19] = 0x00;
    ret = usb_control_msg(mouse->usb_dev,
                          usb_sndctrlpipe(mouse->usb_dev, 0),
                          0x09, 0x21, 0x0211, 1,
                          buf, 20, USB_CTRL_SET_TIMEOUT);
    if (ret < 0) goto done;

    ret = usb_bulk_msg(mouse->usb_dev,
                       usb_rcvbulkpipe(mouse->usb_dev, 0x82),
                       rbuf, 20, NULL, 1000);

done:
    kfree(buf);
    kfree(rbuf);
    return (ret < 0) ? ret : 0;
}

int set_mouse_dpi(const mouse_dev_t *mouse, const int dpi) {
    unsigned char *buf = kzalloc(20, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    buf[0] = 0x10;
    buf[1] = 0xFF;
    buf[2] = 0x0A;
    buf[3] = 0x3B;
    buf[4] = 0x00;
    buf[5] = (dpi >> 8) & 0xFF;
    buf[6] = dpi & 0xFF;

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

long mouse_ioctl(struct file *file, const unsigned int cmd, const unsigned long arg) {
    int ret = 0;
    mouse_dev_t *mouse = file->private_data;
    mutex_lock(&mouse->ioctl_lock);
    switch (cmd) {
        case MOUSE_SET_LEDS: {
            struct led_packet p;
            if (copy_from_user(&p, (struct led_packet __user *) arg, sizeof(struct led_packet))) {
                mutex_unlock(&mouse->ioctl_lock);
                return -EFAULT;
            }
            ret = set_led_colour(mouse, p);
            break;
        }
        case MOUSE_SET_DPI: {
            int dpi_val;
            if (copy_from_user(&dpi_val, (int __user *) arg, sizeof(int))) {
                mutex_unlock(&mouse->ioctl_lock);

                return -EFAULT;
            }
            if (dpi_val < 200 || dpi_val > 8000) return -EINVAL;
            ret = set_mouse_dpi(mouse, dpi_val);
            break;
        }
        default:
            return -ENOTTY;
    }
    mutex_unlock(&mouse->ioctl_lock);

    return ret;
}
