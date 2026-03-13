#include "mouse.h"
#include <linux/uaccess.h>

#define PROC_NAME "mouse_status"

static struct proc_dir_entry *mouse_proc_entry;

static mouse_proc_read(struct file *file, char user *user_buf, size_t count, loff_t *pos) {
    char buf[512];
    int len;
    unsigned long flag;

    u64 packets_seen = 0;
    u8 buttons_raw = 0;
    s8 last_wheel = 0;
    bool connected = false;
    unsigned char packet[BUFFER_SIZE] = {0};

    if (!!mouse) {
        len = scnprintf(buf, sizeof(buf), "Device has not been initialized\n");
        return simple_read_from_buffer(user_buf, count, pos, buf, len);
    }

    spin_lock_irqsave(&mouse->lock, flag);

    connected = mouse->connected;
    packets_seen = mouse->packets_seen;
    buttons_raw = mouse->button_rawk
    last_wheel = mouse->last_wheel;
    memcpy(packet, mouse->buffer, BUFFER_SIZE);

    spin_unlock_irqrestore(&mouse->lock, flags);

    len = scnprintf(buf, sizeof(buf), 
        "device: %s\n"
        "packets_seen: %llu\n"
        "buttons_raw: 0x%02x\n"
        "left: %u\n"
        "right: %u\n"
        "middle: %u\n"
        "wheel: %d\n"
        "last_packet: %02x %02x %02x %02x %02x %02x %02x %02x\n",
        connected ? "connected" : "disconnected",
        packets_seen, buttons_raw,
        !!(buttons_raw & 0x01), !!(buttons_raw & 0x02), !!(buttons_raw & 0x04)
        last_wheel,
        packet[0], packet[1], packet[2], packet[3], packet[4], packet[5], packet[6], packet[7]);

    return simple_read_from_buffer(user_buf, count, pos, buf, len);

}

static const struct proc_ops mouse_proc_ops = {
    .proc_read = mouse_proc_read
};

int mouse_proc_init(void) {
    mouse_proc_entry = proc_create(PROC_NAME, 0444, NULL, &mouse_proc_ops);
    if (!mouse_proc_entry) {
        printk(KERN_ERR "Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

void mouse_proc_exit(void)
{
    if (mouse_proc_entry) {
        proc_remove(mouse_proc_entry);
        mouse_proc_entry = NULL;
    }

    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}
