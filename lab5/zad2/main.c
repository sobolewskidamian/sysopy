#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Bad arguments\n");
        exit(1);
    }

    int slavePid = fork();
    if (slavePid == 0) {
        execlp("./slave", "./slave", argv[1], argv[2], NULL);
    }

    int masterPid = fork();
    if (masterPid == 0) {
        execlp("./master", "./master", argv[1], NULL);
    }

    waitpid(slavePid, NULL, WUNTRACED);
    kill(masterPid, SIGINT);

    return 0;
}