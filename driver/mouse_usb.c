#include "mouse.h"


struct usb_device_id mouse_usb_table[] = {{USB_DEVICE(VENDOR_ID, PRODUCT_ID)}, {}};
MODULE_DEVICE_TABLE(usb, mouse_usb_table);

static void mouse_irq(struct urb *urb) {
    if (urb->status) {
        printk(KERN_ERR "URB error: %d", urb->status);
        return;
    }

    unsigned char *data = urb->transfer_buffer;
    printk("mouse len:%d | %02x %02x %02x %02x %02x %02x %02x %02x",
    urb->actual_length,
    data[0], data[1], data[2], data[3],
    data[4], data[5], data[6], data[7]);

    usb_submit_urb(urb, GFP_ATOMIC);
}

static int mouse_probe(struct usb_interface *intfs, const struct usb_device_id *id) {
    if (intfs->cur_altsetting->desc.bInterfaceNumber != 0)
        return -ENODEV;
    mouse_dev_t *mouse = kzalloc(sizeof(mouse_dev_t), GFP_KERNEL);
    if (!mouse) {
        goto mouse_free;
    }
    mouse->buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!mouse->buffer) {
        goto buffer_free;
    }
    mouse->dev = interface_to_usbdev(intfs);
    struct usb_endpoint_descriptor *ep_des = &intfs->cur_altsetting->endpoint->desc;
    mouse->intfs = intfs;
    usb_set_intfdata(intfs, mouse);
    mouse->urb = usb_alloc_urb(0,GFP_KERNEL);
    if (!mouse->urb) {
        printk(KERN_ERR "Mouse urb not sent");
        goto urb_free;
    }
    usb_fill_int_urb(
        mouse->urb,
        mouse->dev,
        usb_rcvintpipe(mouse->dev, ep_des->bEndpointAddress),
        mouse->buffer,
        BUFFER_SIZE,
        mouse_irq,
        mouse,
        ep_des->bInterval


    );
    int connection;
    connection = usb_submit_urb(mouse->urb,GFP_KERNEL);
    if (connection) {
        printk(KERN_ERR "Mouse urb not sent ");
        goto urb_free;
    }
    printk("MOUSE BOUND");
    return 0;
urb_free:
    usb_free_urb(mouse->urb);
buffer_free:
    kfree(mouse->buffer);
mouse_free:
    kfree(mouse);
    return -ENOMEM;
}

void mouse_disconnect(struct usb_interface *intfs) {
    mouse_dev_t *mouse = usb_get_intfdata(intfs);
    usb_kill_urb(mouse->urb);
    usb_free_urb(mouse->urb);
    kfree(mouse->buffer);
    kfree(mouse);
    printk("MOUSE DISSCONNECTED");
}

struct usb_driver mouse_driver = {
    .name = "MOUSE",
    .id_table = mouse_usb_table,
    .probe = mouse_probe,
    .disconnect = mouse_disconnect
};

