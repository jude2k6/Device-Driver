#include "device_api.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int mouse_open(void) {
    int fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open " DEVICE_PATH);
    }
    return fd;
}

void mouse_close(int fd) {
    close(fd);
}

int mouse_read_data(int fd, unsigned char *buffer) {
    ssize_t n = read(fd, buffer, DATA_SIZE);
    if (n < 0) {
        perror("Failed to read from mouse device");
    }
    return (int)n;
}

/*
 data[0] - button bitmask
 data[1] - reserved
 data[2-3] - X movement (16-bit little-endian, signed)
 data[4-5] - Y movement (16-bit little-endian, signed)
 data[6] - scroll wheel (signed)
 data[7] - reserved
 */
void mouse_print_event(const unsigned char *data) {
    //Buttons (data[0])
    if (data[0] & BTN_LEFT_MASK)    printf("[CLICK]  Left Button\n");
    if (data[0] & BTN_RIGHT_MASK)   printf("[CLICK]  Right Button\n");
    if (data[0] & BTN_MIDDLE_MASK)  printf("[CLICK]  Middle Button\n");
    if (data[0] & BTN_BACK_MASK)    printf("[CLICK]  Side Button - Back\n");
    if (data[0] & BTN_FORWARD_MASK) printf("[CLICK]  Side Button - Forward\n");

    // Movement (data[2-3] = X, data[4-5] = Y)
    short x = (short)(data[2] | (data[3] << 8));
    short y = (short)(data[4] | (data[5] << 8));
    if (x != 0 || y != 0) {
        printf("[MOVE]   X: %-5d  Y: %d\n", x, y);
    }

    // Scroll wheel (data[6], signed)
    signed char scroll = (signed char)data[6];
    if (scroll > 0) printf("[SCROLL] Up   (%d)\n", scroll);
    if (scroll < 0) printf("[SCROLL] Down (%d)\n", scroll);
}