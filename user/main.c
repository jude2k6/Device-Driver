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