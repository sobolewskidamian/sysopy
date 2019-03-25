#define _XOPEN_SOURCE 700

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

char *removeFileName(char *mystr) {
    char *retstr;
    char *lastdot;
    if (mystr == NULL)
        return NULL;
    if ((retstr = malloc(strlen(mystr) + 1)) == NULL)
        return NULL;
    strcpy(retstr, mystr);
    lastdot = strrchr(retstr, '/');
    if (lastdot != NULL)
        *lastdot = '\0';
    return retstr;
}

char *getFileName(char *mystr) {
    char *dot = strrchr(mystr, '/');
    if (!dot || dot == mystr) return "";
    return dot + 1;
}

void createExecutableFile(char *filePath) {
    remove(filePath);
    char *path = removeFileName(filePath);
    char *fileName = getFileName(filePath);
    char *pathTemporary = calloc(strlen(path) + 100, sizeof(char));
    char *bufferToCreateFile = calloc(50 + strlen(path), sizeof(char));
    strcpy(pathTemporary, path);
    strcat(pathTemporary, "/");
    strcat(pathTemporary, fileName);

    char *echo = calloc(100 + strlen(path), sizeof(char));
    char *buffer = calloc(200 + strlen(pathTemporary), sizeof(char));
    strcpy(echo, fileName);
    strcat(echo, "-temporary ");
    strcat(echo, path);
    strcat(echo, " 1");

    sprintf(bufferToCreateFile, "touch %s", pathTemporary);
    sprintf(buffer, "echo '%s' >> %s", echo, pathTemporary);
    system(bufferToCreateFile);
    system(buffer);
}

int main(int argc, char **argv) {
    if (argc != 5 || atoi(argv[2]) < 0 || atoi(argv[3]) <= 0 || atoi(argv[4]) <= 0) {
        printf("Bad arguments");
        return 1;
    }

    srand((unsigned int) time(NULL));
    char *path = argv[1];
    createExecutableFile(path);
    char *pathTemporary = calloc(strlen(path) + 100, sizeof(char));
    char *bufferToCreateFile = calloc(50 + strlen(path), sizeof(char));
    strcpy(pathTemporary, path);
    strcat(pathTemporary, "-temporary");
    sprintf(bufferToCreateFile, "touch %s", pathTemporary);
    system(bufferToCreateFile);

    char *timeForProcessStr = calloc(20, sizeof(char));
    int blockSize = (int) strtol(argv[4], (char **) NULL, 10);
    int time1 = (int) strtol(argv[2], (char **) NULL, 10);
    int time2 = (int) strtol(argv[3], (char **) NULL, 10);
    int timeInSeconds = rand() % (time2 - time1) + time1;
    int timeForProcess = 10;
    sprintf(timeForProcessStr, "%d", timeForProcess);

    pid_t child_pid = fork();
    if (child_pid == 0) {
        char *const av[] = {"./main", path, timeForProcessStr, "TRYB2", NULL};
        execvp("./main", av);
        exit(0);
    }

    testedRunAddToFile(pathTemporary, blockSize, timeInSeconds);
    clock_t start = clock();
    while ((clock() - start) / CLOCKS_PER_SEC < timeForProcess) {
        clock_t actTime = clock();
        while ((clock() - actTime) / CLOCKS_PER_SEC < timeInSeconds) {}
        timeInSeconds = rand() % (time2 - time1 + 1) + time1;
        testedRunAddToFile(pathTemporary, blockSize, timeInSeconds);
    }

    free(pathTemporary);
    free(bufferToCreateFile);
    free(timeForProcessStr);
}