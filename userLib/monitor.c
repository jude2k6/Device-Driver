#include <stdio.h>
#include "device_api.h"

int main(void) {
    unsigned char data[DATA_SIZE];

    int fd = mouse_open();
    if (fd < 0) {
        return 1;
    }

    printf("Mouse device listening for events\n");
    while (1) {
        if (mouse_read_data(fd, data) < 0) {
            break;
        }
        mouse_print_event(data);
    }
    mouse_close(fd);
    return 0;
}

/* OLD VERSION
 #include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    unsigned char data[8];

    int fd = open("/dev/MOUSE", O_RDONLY);


    while (1) {
        ssize_t n = read(fd, data, 8);

        printf("| %02x %02x %02x %02x %02x %02x %02x %02x\n",
               data[0], data[1], data[2], data[3],
               data[4], data[5], data[6], data[7]);
    }

    close(fd);
    return 0;
}

//data is 8 bit from char.c mouse_read, left click instead of printing 0 1 for left click, print left click, right, scroll wheel, side. basically trans;ate the 8 bit into a human readable output.
*/
