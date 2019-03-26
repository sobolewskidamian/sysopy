#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int awaitingHandler = 0;
int isDeadProcess = 0;

void au(int num) {
    if (awaitingHandler == 0) {
        printf("\nOdebrano sygnał %d\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n", num);
    }
    if (awaitingHandler == 0) awaitingHandler = 1;
    else awaitingHandler = 0;
}

void initSignal(int num) {
    printf("\nOdebrano sygnał SIGINT - %d\n", num);
    exit(0);
}

int main() {
    struct sigaction act;
    act.sa_handler = au;
    act.sa_flags = 0;

    pid_t pid = fork();
    if (pid == 0) {
        execl("./date.sh", "./date.sh", NULL);
        exit(0);
    }

    while (1) {
        sigaction(SIGTSTP, &act, NULL);
        signal(SIGINT, initSignal);

        if (awaitingHandler == 0) {
            if (isDeadProcess != 0) {
                isDeadProcess = 0;
                pid = fork();
                if (pid == 0) {
                    printf("\nChild PID: %d\n", getpid());
                    execl("./date.sh", "./date.sh", NULL);
                    exit(0);
                }
            }
        } else {
            if (isDeadProcess == 0) {
                kill(pid, SIGKILL);
                isDeadProcess = 1;
            }
        }
    }
}
