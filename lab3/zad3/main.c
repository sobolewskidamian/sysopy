#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE

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
#include <sys/time.h>

struct files {
    char *name;
    char *path;
    char *times;
};

struct memoryFiles {
    char *memory;
    time_t modification;
};

struct files *filesArr;
struct memoryFiles *memoryFiles;

int getNumberOfLines(FILE *file) {
    int lines = 0;
    char ch;
    while ((ch = (char) fgetc(file)) != EOF) {
        if (ch == '\n')
            lines++;
    }
    fseek(file, 0L, SEEK_SET);
    return lines;
}

long getSizeOfFile(FILE *fp) {
    if (fp == NULL)
        return -1;

    fseek(fp, 0L, SEEK_END);
    long int result = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return result;
}

void copy(char *from, char *to) {
    pid_t child_pid = fork();
    if (child_pid == 0) {
        char *const av[] = {"cp", from, to, NULL};
        execvp("cp", av);
        exit(0);
    }else {
        int statLock = 0;
        wait(&statLock);
    }
}


void monitore2(int *amountOfChanged, char *path, char *name, int times, int timeMainProcess) {
    struct stat fileStat;
    char newPath[PATH_MAX];
    strcpy(newPath, path);
    strcat(newPath, "/");
    strcat(newPath, name);
    lstat(newPath, &fileStat);
    time_t lastUpdate = fileStat.st_mtime;

    char timeArr[80];
    char newPathOfCopy[PATH_MAX];
    struct tm lt;
    mkdir("archiwum", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    strcpy(newPathOfCopy, "archiwum/");
    strcat(newPathOfCopy, name);
    localtime_r(&fileStat.st_mtime, &lt);
    strftime(timeArr, sizeof timeArr, "_%Y-%m-%d_%H-%M-%S", &lt);
    strcat(newPathOfCopy, timeArr);

    copy(newPath, newPathOfCopy);

    clock_t start = clock();
    while ((clock() - start) / CLOCKS_PER_SEC < timeMainProcess) {
        clock_t actTime = clock();
        while ((clock() - actTime) / CLOCKS_PER_SEC < times) {}
        lstat(newPath, &fileStat);
        if (fileStat.st_mtime != lastUpdate) {
            lastUpdate = fileStat.st_mtime;
            strcpy(newPathOfCopy, "archiwum/");
            strcat(newPathOfCopy, name);
            localtime_r(&fileStat.st_mtime, &lt);
            strftime(timeArr, sizeof timeArr, "_%Y-%m-%d_%H-%M-%S", &lt);
            strcat(newPathOfCopy, timeArr);
            copy(newPath, newPathOfCopy);
            (*amountOfChanged)++;
        }
    }
}

void monitore1(int *amountOfChanged, char *path, char *name, int times, time_t modification, char *memory,
               int timeMainProcess) {
    clock_t start = clock();
    while ((clock() - start) / CLOCKS_PER_SEC < timeMainProcess) {
        clock_t actTime = clock();
        while ((clock() - actTime) / CLOCKS_PER_SEC < times) {}

        struct stat fileStat;
        char newPath[PATH_MAX];
        strcpy(newPath, path);
        strcat(newPath, "/");
        strcat(newPath, name);
        lstat(newPath, &fileStat);
        if (modification != fileStat.st_mtime) {
            (*amountOfChanged)++;
            char timeArr[80];
            char newPathToSave[PATH_MAX];
            struct tm lt;

            mkdir("archiwum", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            strcpy(newPathToSave, "archiwum/");
            strcat(newPathToSave, name);
            localtime_r(&modification, &lt);
            strftime(timeArr, sizeof timeArr, "_%Y-%m-%d_%H-%M-%S", &lt);
            strcat(newPathToSave, timeArr);
            FILE *file = fopen(newPathToSave, "w");
            fputs(memory, file);
            fclose(file);
            modification = fileStat.st_mtime;

            FILE *file2 = fopen(newPath, "r");
            long int size = getSizeOfFile(file2);
            memory = calloc((size_t) size, sizeof(char));
            int iter = 0;
            char ch;
            while ((ch = (char) fgetc(file2)) != EOF)
                memory[iter++] = ch;
            fclose(file2);
        }
    }
}

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

void limits(char *time, char *memory) {
    long int timeLimit = strtol(time, (char **) NULL, 10);
    struct rlimit rlimitCPU;
    rlimitCPU.rlim_max = (rlim_t) timeLimit;
    rlimitCPU.rlim_cur = (rlim_t) timeLimit;
    if (setrlimit(RLIMIT_CPU, &rlimitCPU) != 0) {
        printf("Can't set cpu limits.");
        return;
    }

    long int memoryLimit = strtol(memory, (char **) NULL, 10) * 1024 * 1024;
    struct rlimit rlimitMemory;
    rlimitMemory.rlim_max = (rlim_t) memoryLimit;
    rlimitMemory.rlim_cur = (rlim_t) memoryLimit;
    if (setrlimit(RLIMIT_AS, &rlimitMemory) != 0) {
        printf("Can't set memory limits.");
        return;
    }
}


int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Bad arguments");
        return 1;
    }
    if (strcmp(argv[1], "MONITORE") == 0) {
        int amountOfChanged = 0;
        if (argc == 9 && strcmp(argv[8], "1") == 0) {
            monitore1(&amountOfChanged, argv[2], argv[3], (int) strtol(argv[4], (char **) NULL, 10),
                      strtol(argv[5], (char **) NULL, 10), argv[6], (int) strtol(argv[7], (char **) NULL, 10));
        } else if (argc == 7 && strcmp(argv[6], "2") == 0) {
            monitore2(&amountOfChanged, argv[2], argv[3], (int) strtol(argv[4], (char **) NULL, 10),
                      (int) strtol(argv[5], (char **) NULL, 10));
        } else exit(-1);
        exit(amountOfChanged);
    } else if (strcmp(argv[1], "RUN_TESTER") == 0) {
        if (argc != 6 || atoi(argv[3]) < 0 || atoi(argv[4]) <= 0 || atoi(argv[5]) <= 0) {
            printf("Bad arguments");
            return 1;
        }

        srand((unsigned int) time(NULL));
        char *path = argv[2];
        char *pathTemporary = calloc(strlen(path) + 100, sizeof(char));
        char *bufferToCreateFile = calloc(50 + strlen(path), sizeof(char));
        strcpy(pathTemporary, path);
        strcat(pathTemporary, "-temporary");
        sprintf(bufferToCreateFile, "touch %s", pathTemporary);
        system(bufferToCreateFile);

        char *timeForProcessStr = calloc(20, sizeof(char));
        int blockSize = (int) strtol(argv[5], (char **) NULL, 10);
        int time1 = (int) strtol(argv[3], (char **) NULL, 10);
        int time2 = (int) strtol(argv[4], (char **) NULL, 10);
        int timeInSeconds = rand() % (time2 - time1) + time1;
        int timeForProcess = 10;
        sprintf(timeForProcessStr, "%d", timeForProcess);

        pid_t child_pid = fork();
        if (child_pid == 0) {
            char *const av[] = {argv[0], path, timeForProcessStr, "TRYB1", "20", "20", NULL};
            execvp(argv[0], av);
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
    } else {
        if (argc != 6 || atoi(argv[2]) <= 0 || atoi(argv[4]) <= 0 || atoi(argv[5]) <= 0) {
            printf("Bad arguments");
            return 1;
        }

        FILE *file = fopen(argv[1], "r");
        if (!file) {
            printf("Can't open a file");
            return 1;
        }
        int numberOfLines = getNumberOfLines(file);
        filesArr = calloc((size_t) numberOfLines, sizeof(struct files));
        struct rusage prevRusage;
        getrusage(RUSAGE_CHILDREN, &prevRusage);

        for (int i = 0; i < numberOfLines; i++) {
            filesArr[i].name = calloc(256, sizeof(char));
            filesArr[i].path = calloc(256, sizeof(char));
            filesArr[i].times = calloc(15, sizeof(char));
        }

        char ch;
        int iterPath = 0;
        int iterName = 0;
        int iterTime = 0;
        int iterArg = 0;
        int line = 0;
        while ((ch = (char) fgetc(file)) != EOF) {
            if (ch == '\n') {
                if (iterArg != 2 || iterTime == 0) {
                    printf("Bad arguments in file");
                    return 1;
                }

                iterArg = iterName = iterPath = iterTime = 0;
                line++;
            } else if (ch == ' ') {
                iterName = iterPath = iterTime = 0;
                iterArg++;
            } else if (iterArg == 0) {
                filesArr[line].name[iterName++] = ch;
            } else if (iterArg == 1) {
                filesArr[line].path[iterPath++] = ch;
            } else if (iterArg == 2) {
                filesArr[line].times[iterTime++] = ch;
            }
        }
        fclose(file);

        memoryFiles = calloc((size_t) numberOfLines, sizeof(struct memoryFiles));
        struct stat fileStat;
        for (int i = 0; i < numberOfLines; i++) {
            char newPath[PATH_MAX];
            strcpy(newPath, filesArr[i].path);
            strcat(newPath, "/");
            strcat(newPath, filesArr[i].name);
            FILE *file = fopen(newPath, "r");
            if (!file) {
                printf("Can't open a file");
                return 1;
            }

            lstat(newPath, &fileStat);
            memoryFiles[i].modification = fileStat.st_mtime;
            long int size = getSizeOfFile(file);
            memoryFiles[i].memory = calloc((size_t) size, sizeof(char));
            int iter = 0;
            while ((ch = (char) fgetc(file)) != EOF) {
                memoryFiles[i].memory[iter++] = ch;
            }
            fclose(file);
        }

        for (int i = 0; i < numberOfLines; i++) {
            pid_t child_pid = fork();
            if (child_pid == 0) {
                char modificationTime[32];
                sprintf(modificationTime, "%ld", memoryFiles[i].modification);
                limits(argv[4], argv[5]);
                if (strcmp(argv[3], "TRYB1") == 0) { //tryb1
                    char *const av[] = {argv[0], "MONITORE", filesArr[i].path, filesArr[i].name, filesArr[i].times,
                                        modificationTime,
                                        memoryFiles[i].memory, argv[2], "1", NULL};
                    execvp(argv[0], av);
                    return -1;
                } else { //tryb2
                    char *const av[] = {argv[0], "MONITORE", filesArr[i].path, filesArr[i].name, filesArr[i].times,
                                        argv[2], "2", NULL};
                    execvp(argv[0], av);
                    return -1;
                }
            }
        }

        for (int i = 0; i < numberOfLines; i++) {
            int s = 0;
            pid_t finished;
            finished = waitpid(-1, &s, 0);
            s = s >> 8;

            struct rusage rusage;
            getrusage(RUSAGE_CHILDREN, &rusage);
            struct timeval ru_utime;
            struct timeval ru_stime;
            timersub(&rusage.ru_utime, &prevRusage.ru_utime, &ru_utime);
            timersub(&rusage.ru_stime, &prevRusage.ru_stime, &ru_stime);
            prevRusage = rusage;
            printf("\n");
            printf("User time: %d.%d seconds\nSystem time: %d.%d seconds\n\n", (int) ru_utime.tv_sec,
                   (int) ru_utime.tv_usec, (int) ru_stime.tv_sec, (int) ru_stime.tv_usec);
            printf("Proces %d utworzył %d kopii pliku\n", finished, s);
        }


        for (int i = 0; i < numberOfLines; i++) {
            free(filesArr[i].name);
            free(filesArr[i].path);
            free(filesArr[i].times);
        }

        free(filesArr);
        free(memoryFiles);
    }
}