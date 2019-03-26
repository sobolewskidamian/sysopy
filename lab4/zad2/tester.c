#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

void testedRunAddToFile(char *path, int blockSize, int seconds) {
    char timeArr[80];
    time_t rawtime = time(NULL);
    struct tm *ptm = localtime(&rawtime);
    strftime(timeArr, 80, "%Y-%m-%d %H:%M:%S", ptm);
    char *commandBufferFirst = calloc(200 + strlen(path), sizeof(char));
    char *commandBuffer = calloc(200 + strlen(path), sizeof(char));
    sprintf(commandBufferFirst, "echo '\n\nPID: %d\nSECONDS: %d\nDATE: %s' >> %s", getpid(), seconds, timeArr, path);
    sprintf(commandBuffer, "cat /dev/urandom | base64 | head -c %d >> %s", blockSize, path);

    system(commandBufferFirst);
    system(commandBuffer);

    free(commandBufferFirst);
    free(commandBuffer);
}

int main(int argc, char **argv) {
    if (argc != 5 || atoi(argv[2]) < 0 || atoi(argv[3]) <= 0 || atoi(argv[4]) <= 0) {
        printf("Bad arguments");
        return 1;
    }

    srand((unsigned int) time(NULL));
    char *path = argv[1];
    char *bufferToCreateFile = calloc(50 + strlen(path), sizeof(char));
    sprintf(bufferToCreateFile, "touch %s", path);
    system(bufferToCreateFile);

    int blockSize = (int) strtol(argv[4], (char **) NULL, 10);
    int time1 = (int) strtol(argv[2], (char **) NULL, 10);
    int time2 = (int) strtol(argv[3], (char **) NULL, 10);
    int timeInSeconds = rand() % (time2 - time1) + time1;

    testedRunAddToFile(path, blockSize, timeInSeconds);
    while (1) {
        clock_t actTime = clock();
        while ((clock() - actTime) / CLOCKS_PER_SEC < timeInSeconds) {}
        timeInSeconds = rand() % (time2 - time1 + 1) + time1;
        testedRunAddToFile(path, blockSize, timeInSeconds);
    }

    free(path);
    free(bufferToCreateFile);
}