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

struct files {
    char *name;
    char *path;
    char *times;
    int pid;
};

struct memoryFiles {
    char *memory;
    time_t modification;
};

struct files *filesArr;
struct memoryFiles *memoryFiles;
int amountOfChanged = 0;
int monitoreProcess = 1;

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

void signalHandler(int sig) {
    if (sig == SIGUSR1)
        monitoreProcess = 0;
    else if (sig == SIGUSR2)
        monitoreProcess = 1;
}

void printRaport() {
    printf("Proces %d utworzył %d kopii pliku\n", getpid(), amountOfChanged);
    exit(0);
}

void monitore(int *amountOfChanged, char *path, char *name, int times, time_t modification, char *memory) {
    while (1) {
        signal(SIGUSR1, signalHandler);
        signal(SIGUSR2, signalHandler);
        signal(SIGINT, printRaport);

        clock_t actTime = clock();
        while ((clock() - actTime) / CLOCKS_PER_SEC < times) {}

        struct stat fileStat;
        char newPath[PATH_MAX];
        strcpy(newPath, path);
        strcat(newPath, "/");
        strcat(newPath, name);
        lstat(newPath, &fileStat);
        if (monitoreProcess == 1 && modification != fileStat.st_mtime) {
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

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Bad arguments");
        return 1;
    }
    if (strcmp(argv[1], "MONITORE") == 0) {
        monitore(&amountOfChanged, argv[2], argv[3], (int) strtol(argv[4], (char **) NULL, 10),
                 strtol(argv[5], (char **) NULL, 10), argv[6]);

        exit(amountOfChanged);
    } else {
        if (argc < 2) {
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
                printf("%d: %s/%s\n", getpid(), filesArr[i].path, filesArr[i].name);
                char *const av[] = {argv[0], "MONITORE", filesArr[i].path, filesArr[i].name, filesArr[i].times,
                                    modificationTime,
                                    memoryFiles[i].memory, NULL};
                execvp(argv[0], av);
                return -1;
            } else {
                filesArr[i].pid = child_pid;
            }
        }

        while (1) {
            char data[64];
            fgets(data, sizeof data, stdin);

            if (strcmp(data, "LIST\n") == 0) {
                for (int i = 0; i < numberOfLines; i++)
                    printf("%d\n", filesArr[i].pid);
            } else if (strcmp(data, "STOP ALL\n") == 0) {
                for (int i = 0; i < numberOfLines; i++)
                    kill(filesArr[i].pid, SIGUSR1);
            } else if (strcmp(data, "START ALL\n") == 0) {
                for (int i = 0; i < numberOfLines; i++)
                    kill(filesArr[i].pid, SIGUSR2);
            } else if (strcmp(data, "END\n") == 0) {
                for (int i = 0; i < numberOfLines; i++)
                    kill(filesArr[i].pid, SIGINT);
                break;
            } else {
                char *firstArg = strtok(data, " ");
                char *secondArg;
                if (firstArg != NULL)
                    secondArg = strtok(NULL, " ");

                int pid = (int) strtol(secondArg, (char **) NULL, 10);
                if (pid < 0)
                    continue;

                if (strcmp(firstArg, "START") == 0)
                    kill(pid, SIGUSR2);
                else if (strcmp(firstArg, "STOP") == 0)
                    kill(pid, SIGUSR1);
            }
        }

        for (int i = 0; i < numberOfLines; i++) {
            free(filesArr[i].name);
            free(filesArr[i].path);
            free(filesArr[i].times);
        }
    }
}
