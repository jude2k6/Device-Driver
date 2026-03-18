#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../include/shared_ioctl.h"

int main(int argc, char *argv[]) {
    // 1. Open with O_RDWR (Read/Write)
    int fd = open("/dev/MOUSE", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/MOUSE (Are you sudo?)");
        return 1;
    }

    struct led_packet p = {
        .mode = 0x01,
        .r = 1,
        .g = 1,
        .b = 255
    };

    // 2. Check the return value of ioctl
    if (ioctl(fd, MOUSE_SET_LEDS, &p) < 0) {
        perror("IOCTL MOUSE_SET_LEDS failed");
        close(fd);
        return 1;
    }



    printf("LED command sent successfully!\n");

    close(fd);
    return 0;
}