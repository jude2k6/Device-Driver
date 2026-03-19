#ifndef DEVICE_API_H
#define DEVICE_API_H
#include <stddef.h>

#define DEVICE_PATH "/dev/MOUSE"
#define DATA_SIZE   8

//Button bit masks from data[0]
#define BTN_LEFT_MASK    0x01
#define BTN_RIGHT_MASK   0x02
#define BTN_MIDDLE_MASK  0x04
#define BTN_BACK_MASK    0x08
#define BTN_FORWARD_MASK 0x10

int mouse_open(void);

void mouse_close(int fd);

int mouse_read_data(int fd, unsigned char *buffer);

void mouse_print_event(const unsigned char *data);

#endif // DEVICE_API_H
