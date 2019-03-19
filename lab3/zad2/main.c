#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>

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

void monitore(int index, int *amountOfChanged) {
    struct stat fileStat;
    char newPath[PATH_MAX];
    strcpy(newPath, filesArr[index].path);
    strcat(newPath, "/");
    strcat(newPath, filesArr[index].name);
    lstat(newPath, &fileStat);
    if (memoryFiles[index].modification != fileStat.st_mtime) {
        amountOfChanged++;
        char timeArr[80];
        char newPathToSave[PATH_MAX];
        struct tm lt;

        mkdir("archiwum", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        strcpy(newPathToSave, "archiwum/");
        strcat(newPathToSave, filesArr[index].name);
        localtime_r(&memoryFiles[index].modification, &lt);
        strftime(timeArr, sizeof timeArr, "_%Y-%m-%d_%H-%M-%S", &lt);
        strcat(newPathToSave, timeArr);
        FILE *file = fopen(newPathToSave, "w");
        fputs(memoryFiles[index].memory, file);
        fclose(file);

        FILE *file2 = fopen(newPath, "r");
        long int size = getSizeOfFile(file2);
        memoryFiles[index].memory = calloc((size_t) size, sizeof(char));
        int iter = 0;
        char ch;
        while ((ch = (char) fgetc(file2)) != EOF)
            memoryFiles[index].memory[iter++] = ch;
        fclose(file2);
    }
}


int main(int argc, char **argv) {
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

        //pid_t pid = fork();
    }

    //test
    FILE *filee = fopen("/home/damian/Pulpit/Sysopy/lab3/zad2/plik2", "w");
    fputs("safsafw\nabc\ndefdsad\ndgdsg", filee);
    fclose(filee);
    monitore(0, 0);
    printf("%s", memoryFiles[0].memory);


    for (int i = 0; i < numberOfLines; i++) {
        free(filesArr[i].name);
        free(filesArr[i].path);
        free(filesArr[i].times);
        free(memoryFiles[i].memory);
    }
    free(filesArr);
    free(memoryFiles);
    return 0;
}