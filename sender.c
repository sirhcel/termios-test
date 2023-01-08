#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>


char const * const PATTERN = " 0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";


int main(int argc, char **argv) {
    int result = -1;

    printf("hello serial afficionados!\n");

    if (argc < 2) {
        fprintf(stderr, "device name missing\n");
        exit(1);
    }

    printf("device: %s\n", argv[1]);
    printf("about to open ...\n");
    const int fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "opening %s failed: %s (%d)\n", argv[1], strerror(errno), errno);
        exit(1);
    }

    struct termios config = { 0 };
    printf("tcgetattr ...\n");
    result = tcgetattr(fd, &config);
    if (result < 0) {
        fprintf(stderr, "tcgetattr failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }

    printf("cfmakeraw ...\n");
    cfmakeraw(&config);

    config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | INPCK | IGNPAR | IGNCR | ICRNL | IXON | IXOFF);
    config.c_oflag &= ~OPOST;
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    config.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);

    config.c_cflag |= (CS8 | PARENB | CREAD);
    config.c_cflag |= CRTSCTS;

    printf("cfsetispeed ...\n");
    result = cfsetispeed(&config, 115200);
    if (result < 0) {
        fprintf(stderr, "cfsetispeed failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }
    printf("cfsetospeed ...\n");
    result = cfsetospeed(&config, 115200);
    if (result < 0) {
        fprintf(stderr, "cfsetospeed failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }

    printf("tcsetattr ...\n");
    result = tcsetattr(fd, TCSANOW, &config);
    if (result < 0) {
        fprintf(stderr, "tcsetattr failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }

    printf("about to transmit ...\n");

    const size_t length = strlen(PATTERN);
    size_t count = 0;
    while (true) {
        const ssize_t written = write(fd, PATTERN, length);
        if (written != length) {
            fprintf(stderr, "failed or short write: %ld (%s (%d))\n", written, strerror(errno), errno);
            exit(1);
        }

        count += 1;
        if (count % 100 == 0) {
            printf("chunk %ld\n", count / 100);
        }
    }
}
