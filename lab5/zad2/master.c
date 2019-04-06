#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        exit(1);
    }

    char *path = argv[1];
    mkfifo(path, S_IWUSR | S_IRUSR);
    int fifo = open(path, O_RDWR);
    char buffer[256];

    while (1) {
        int bytes = read(fifo, buffer, 256);
        if (bytes > 0) {
            printf("%s", buffer);
            memset(buffer, 0, 256);
        }
    }

    return 0;
}