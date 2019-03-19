#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <values.h>
#include <ftw.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

time_t dateUsr;

void print(char *path, const struct stat *fileStat) {
    printf("%s\nType:\t\t\t", path);
    if (S_ISREG(fileStat->st_mode)) printf("reg");
    else if (S_ISDIR(fileStat->st_mode)) printf("dir");
    else if (S_ISCHR(fileStat->st_mode)) printf("char dev");
    else if (S_ISBLK(fileStat->st_mode)) printf("block dev");
    else if (S_ISFIFO(fileStat->st_mode)) printf("fifo");
    else if (S_ISLNK(fileStat->st_mode)) printf("slink");
    else if (S_ISSOCK(fileStat->st_mode)) printf("sock");
    printf("\nSize:\t\t\t%lld\n", (long long int) fileStat->st_size);
    printf("Access time:\t\t%s", ctime(&fileStat->st_atime));
    printf("Modification time:\t%s", ctime(&fileStat->st_mtime));
    printf("\n");
}

void dir_func(char *path, char *operation) {
    DIR *dir = opendir(path);

    if (dir == NULL) {
        //printf("Can't open a path");
        return;
    }

    struct dirent *dirent = readdir(dir);
    struct stat fileStat;
    char newPath[PATH_MAX];

    while (dirent != NULL) {
        strcpy(newPath, path);
        strcat(newPath, "/");
        strcat(newPath, dirent->d_name);
        lstat(newPath, &fileStat);
        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
            dirent = readdir(dir);
        } else {
            int resTime = (int) difftime(dateUsr, fileStat.st_mtime);
            if ((resTime == 0 && strcmp(operation, "=") == 0)
                || (resTime > 0 && strcmp(operation, "<") == 0)
                || (resTime < 0 && strcmp(operation, ">") == 0)) {
                //print(newPath, &fileStat);

                pid_t child_pid = fork();
                if (child_pid == 0) {
                    if (S_ISDIR(fileStat.st_mode)) {
                        printf("\n%s\n", newPath);

                        printf("PID: %d\n", (int) getpid());
                        char *const av[] = {"ls", "-l", newPath, NULL};
                        execvp("ls", av);
                        //printf("\n\n\n");
                    }
                    exit(0);
                }
                int *statLock = 0;
                wait(statLock);
            }

            if (S_ISDIR(fileStat.st_mode)) {
                dir_func(newPath, operation);
            }

            dirent = readdir(dir);
        }
    }
    closedir(dir);
    free(dirent);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Bad arguments");
        return 1;
    }

    char *path = argv[1];
    char *operation = argv[2];
    char *date = argv[3];

    struct tm *timestamp = malloc(sizeof(struct tm));
    strptime(date, "%Y-%m-%d %H:%M:%S", timestamp);
    dateUsr = mktime(timestamp);

    if ((long long int) dateUsr < 0) {
        printf("Bad format of date");
        return 1;
    }

    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("Given path isn't a directory");
        return 1;
    }
    closedir(dir);

    dir_func(realpath(path, NULL), operation);

    free(timestamp);
    return 0;
}