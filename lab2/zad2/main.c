#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <values.h>
#include <ftw.h>

time_t dateUsr;
char *operationForNftw;

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
        printf("Can't open a path");
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
                print(newPath, &fileStat);
            }
            if (S_ISDIR(fileStat.st_mode))
                dir_func(newPath, operation);

            dirent = readdir(dir);
        }
    }
    closedir(dir);
    free(dirent);
}

static int nftw_function(char *fpath, const struct stat *sb) {
    int resTime = (int) difftime(dateUsr, sb->st_mtime);
    if ((resTime == 0 && strcmp(operationForNftw, "=") == 0)
        || (resTime > 0 && strcmp(operationForNftw, "<") == 0)
        || (resTime < 0 && strcmp(operationForNftw, ">") == 0)) {
        print(fpath, sb);
    }
    return 0;
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

    DIR *dir = opendir(realpath(path, NULL));
    if (dir == NULL) {
        printf("Given path isn't a directory");
        return 1;
    }
    closedir(dir);

    dir_func(realpath(path, NULL), operation);

    for (int i = 0; i < 5; i++)
        printf("--------------------------------------------------------\n");

    operationForNftw = operation;
    nftw(realpath(path, NULL), (__nftw_func_t) nftw_function, 10, FTW_PHYS);

    free(timestamp);
    return 0;
}