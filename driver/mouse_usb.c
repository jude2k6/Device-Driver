#include "mouse.h"


struct usb_device_id mouse_usb_table[] = {{USB_DEVICE(VENDOR_ID, PRODUCT_ID)}, {}};
MODULE_DEVICE_TABLE(usb, mouse_usb_table);

static void process_data(const mouse_dev_t *mouse, const unsigned char *data) {
    s16 x = (s16) (data[2] | (data[3] << 8));
    s16 y = (s16) (data[4] | (data[5] << 8));

    // regular input subsystem
    input_report_key(mouse->input_dev, BTN_LEFT, data[0] & 0x01);
    input_report_key(mouse->input_dev, BTN_RIGHT, data[0] & 0x02);
    input_report_key(mouse->input_dev, BTN_MIDDLE, data[0] & 0x04);
    input_report_key(mouse->input_dev, BTN_FORWARD, data[0] & 0x10);
    input_report_key(mouse->input_dev, BTN_BACK, data[0] & 0x08);
    input_report_rel(mouse->input_dev, REL_WHEEL, (s8) data[6]);
    input_report_rel(mouse->input_dev, REL_X, x);
    input_report_rel(mouse->input_dev, REL_Y, y);
    input_sync(mouse->input_dev);
}


static int init_input_dev(mouse_dev_t *mouse) {
    mouse->input_dev = input_allocate_device();
    mouse->input_dev->name = "G203 Mouse";
    mouse->input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
    set_bit(BTN_LEFT, mouse->input_dev->keybit);
    set_bit(BTN_RIGHT, mouse->input_dev->keybit);
    set_bit(BTN_MIDDLE, mouse->input_dev->keybit);
    set_bit(BTN_BACK, mouse->input_dev->keybit);
    set_bit(BTN_FORWARD, mouse->input_dev->keybit);
    set_bit(REL_WHEEL, mouse->input_dev->relbit);
    set_bit(REL_X, mouse->input_dev->relbit);
    set_bit(REL_Y, mouse->input_dev->relbit);
    return input_register_device(mouse->input_dev);
}

static void mouse_irq(struct urb *urb) {
    if (urb->status) {
        printk(KERN_ERR "URB error: %d", urb->status);
        return;
    }
    mouse_dev_t *mouse = urb->context;

    unsigned char *data = urb->transfer_buffer;
    memcpy(mouse->buffer, data, BUFFER_SIZE);
    mouse->read_ready = true;
    printk("mouse len:%d | %02x %02x %02x %02x %02x %02x %02x %02x",
           urb->actual_length,
           mouse->buffer[0], mouse->buffer[1], mouse->buffer[2], mouse->buffer[3],
           mouse->buffer[4], mouse->buffer[5], mouse->buffer[6], mouse->buffer[7]);
    process_data(mouse, data);
    usb_submit_urb(urb, GFP_ATOMIC);
    wake_up_interruptible(&mouse->read_queue);
}

static int mouse_probe(struct usb_interface *intfs, const struct usb_device_id *id) {
    // Original value was 0, changed to 1 to handle the correct interface for bluetooth reciever
    if (intfs->cur_altsetting->desc.bInterfaceNumber != 1)
        return -ENODEV;
    mouse_dev_t *mouse = kzalloc(sizeof(mouse_dev_t), GFP_KERNEL);
    if (!mouse) {
        return -1;
    }
    mouse->buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!mouse->buffer) {
        goto buffer_free;
    }
    mouse->usb_dev = interface_to_usbdev(intfs);
    struct usb_endpoint_descriptor *ep_des = &intfs->cur_altsetting->endpoint->desc;
    mouse->intfs = intfs;
    usb_set_intfdata(intfs, mouse);
    init_waitqueue_head(&mouse->read_queue);
    mouse->urb = usb_alloc_urb(0,GFP_KERNEL);


    if (!mouse->urb) {
        printk(KERN_ERR "Mouse urb not sent");
        goto urb_free;
    }
    if (init_input_dev(mouse)) {
        printk(KERN_ERR "ERROR input_register_device");
        goto input_free;
    }
    usb_fill_int_urb(
        mouse->urb,
        mouse->usb_dev,
        usb_rcvintpipe(mouse->usb_dev, ep_des->bEndpointAddress),
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
    if (mouse_char_init(mouse)) {
        printk("Error registering char");
        goto char_fail;
    }

    if (mouse_proc_init(mouse)) {
        printk("Error registering proc");
        goto proc_fail;;
    }


    printk("MOUSE BOUND");
    return 0;
proc_fail:
    mouse_char_exit(mouse);
char_fail:
    usb_kill_urb(mouse->urb);
input_free:
    input_unregister_device(mouse->input_dev);
urb_free:
    usb_free_urb(mouse->urb);
buffer_free:
    kfree(mouse->buffer);
    kfree(mouse);
    return -ENOMEM;
}

void mouse_disconnect(struct usb_interface *intfs) {
    mouse_dev_t *mouse = usb_get_intfdata(intfs);
    input_unregister_device(mouse->input_dev);
    usb_kill_urb(mouse->urb);
    usb_free_urb(mouse->urb);
    mouse_char_exit(mouse);
    mouse_proc_exit(mouse);
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

