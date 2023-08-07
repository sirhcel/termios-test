#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>


char const * const PATTERN = " 0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";


void print_termios(const struct termios *config) {
    printf("    c_iflag: 0x%08lx\n", config->c_iflag);
    printf("    c_oflag: 0x%08lx\n", config->c_oflag);
    printf("    c_cflag: 0x%08lx\n", config->c_cflag);
    printf("    c_lflag: 0x%08lx\n", config->c_lflag);
    printf("    c_cc:");
    for (size_t i = 0; i < NCCS; ++i) {
        printf(" 0x%02x", config->c_cc[i]);
    }
    printf("\n");
    printf("    c_ispeed: %lu\n", config->c_ispeed);
    printf("    c_ospeed: %lu\n", config->c_ospeed);
}


int main(int argc, char **argv) {
    int result = -1;

    printf("hello serial afficionados!\n");

    if (argc < 2) {
        fprintf(stderr, "device name missing\n");
        exit(1);
    }

    printf("device: %s\n", argv[1]);
    printf("about to open ...\n");
    const int fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
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
    print_termios(&config);

    printf("cfmakeraw ...\n");
    cfmakeraw(&config);
    print_termios(&config);

    printf("configuring flags ...\n");
    config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | INPCK | IGNPAR | IGNCR | ICRNL | IXON | IXOFF);
    config.c_oflag &= ~OPOST;
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    config.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
    config.c_cflag |= (CS8 | PARENB | CREAD);
    config.c_cflag |= CRTSCTS;
    config.c_cflag |= CLOCAL;
    print_termios(&config);

    printf("cfsetispeed ...\n");
    result = cfsetispeed(&config, B115200);
    if (result < 0) {
        fprintf(stderr, "cfsetispeed failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }
    print_termios(&config);

    printf("cfsetospeed ...\n");
    result = cfsetospeed(&config, B115200);
    if (result < 0) {
        fprintf(stderr, "cfsetospeed failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }
    print_termios(&config);

    printf("tcsetattr ...\n");
    result = tcsetattr(fd, TCSANOW, &config);
    if (result < 0) {
        fprintf(stderr, "tcsetattr failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }
    print_termios(&config);

    printf("tcgetattr for checking status ...\n");
    result = tcgetattr(fd, &config);
    if (result < 0) {
        fprintf(stderr, "tcgetattr failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }
    print_termios(&config);

    printf("about to transmit ...\n");

    const size_t length = strlen(PATTERN);
    size_t count = 0;
    while (true) {
        size_t written = 0;

        while (written < length) {
            const ssize_t write_result = write(fd, PATTERN, length - written);

            if (write_result < 0) {
                fprintf(stderr, "failed write: %ld (%s (%d))\n", write_result, strerror(errno), errno);
                exit(1);
            } else {
                written += write_result;
            }
        }

        result = tcdrain(fd);
        if (result != 0) {
            fprintf(stderr, "failed to tcdrain: %s (%d)\n", strerror(errno), errno);
            exit(1);
        }

        count += 1;
        if (count % 100 == 0) {
            printf("chunk %ld\n", count / 100);
        }
    }
}
