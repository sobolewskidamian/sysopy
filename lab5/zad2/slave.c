#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        exit(1);
    }

    int fifo = open(argv[1], O_RDWR);
    char buffer1[256];
    char buffer2[280];
    int N = (int) strtol(argv[2], NULL, 10);
    srand((unsigned int) time(NULL));

    for (int i = 0; i < N; i++) {
        sleep((unsigned int) (rand() % (5 - 2) + 2));
        FILE *date = popen("date", "r");
        fread(buffer1, sizeof(char), 256, date);
        pclose(date);

        sprintf(buffer2, "PID: %d, DATE: %s", getpid(), buffer1);
        write(fifo, buffer2, 256);
    }
    close(fifo);

    return 0;
}