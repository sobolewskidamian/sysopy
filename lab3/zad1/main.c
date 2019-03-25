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

void dir_func(char *path, char *subPath) {
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
        char *subPath2 = calloc(strlen(subPath) + strlen(dirent->d_name) + 3, sizeof(char));

        strcpy(subPath2, subPath);
        strcat(subPath2, "/");
        strcat(subPath2, dirent->d_name);
        lstat(newPath, &fileStat);
        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
            dirent = readdir(dir);
        } else {
            pid_t child_pid = fork();
            if (child_pid == 0) {
                if (S_ISDIR(fileStat.st_mode)) {
                    printf("\n%s\n", subPath2);

                    printf("PID: %d\n", (int) getpid());
                    char *const av[] = {"ls", "-l", newPath, NULL};
                    execvp("ls", av);
                }
                exit(0);
            }
            int statLock = 0;
            wait(&statLock);

            if (S_ISDIR(fileStat.st_mode)) {
                dir_func(newPath, subPath2);
            }

            dirent = readdir(dir);
        }
    }
    closedir(dir);
    free(dirent);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Bad arguments");
        return 1;
    }

    char *path = argv[1];

    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("Given path isn't a directory");
        return 1;
    }
    closedir(dir);

    dir_func(realpath(path, NULL), "");
    return 0;
}