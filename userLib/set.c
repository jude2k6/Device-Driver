#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "../include/shared_ioctl.h"


int main(const int argc, char *argv[]) {
    int fd = open("/dev/MOUSE", O_RDWR);
    if (fd < 0) {
        perror("failed to open cdev");
        return 1;
    }

    int ret = 0;

    if (strcmp(argv[1], "colour") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Error: 'colour' requires r g b arguments\n");
            ret = 1;
            goto done;
        }

        struct led_packet p = {
            .r = (unsigned char) atoi(argv[2]),
            .g = (unsigned char) atoi(argv[3]),
            .b = (unsigned char) atoi(argv[4]),
        };

        if (ioctl(fd, MOUSE_SET_LEDS, &p) < 0) {
            perror("set leds failed");
            ret = 1;
            goto done;
        }
        printf("Colour set to r=%d g=%d b=%d\n", p.r, p.g, p.b);
    } else if (strcmp(argv[1], "dpi") == 0) {
        if (argc != 3) {
            fprintf(stderr, "dpi requires a value argument\n");
            ret = 1;
            goto done;
        }

        int dpi = atoi(argv[2]);
        if (dpi < 200 || dpi > 8000) {
            fprintf(stderr, "dpi must be between 200 and 8000\n");
            ret = 1;
            goto done;
        }

        if (ioctl(fd, MOUSE_SET_DPI, &dpi) < 0) {
            perror("dpi set failed");
            ret = 1;
            goto done;
        }
        printf("DPI set to %d\n", dpi);
    } else {
        fprintf(stderr, " unknown command '%s'\n", argv[1]);
        ret = 1;
    }

done:
    close(fd);
    return ret;
}
