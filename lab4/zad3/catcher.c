//
// Created by damian on 27.03.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int flagStopped = 0;
int counter = 0;
int senderPid = 0;

void signalHandler(int sig, siginfo_t *info, void *vp) {
    if (sig == SIGUSR1) {
        counter++;
    } else if (sig == SIGUSR2) {
        flagStopped = 1;
    }
    senderPid = info->si_pid;
}

void sendSignal(int pid, int flag, char *mode) {
    if (strcmp(mode, "KILL") == 0)
        kill(pid, flag);
}

int main(int argc, char **argv) {
    if (argc < 3 ||
        (strcmp(argv[2], "KILL") != 0 && strcmp(argv[2], "SIGQUEUE") != 0 && strcmp(argv[2], "SIGRT") != 0)) {
        printf("Bad arguments\n");
        return 1;
    }
    char *amountOfSignals = argv[1];
    char *mode = argv[2];
    printf("Catcher PID: %d\n", getpid());

    char *pidCharArr = calloc(8, sizeof(char));
    sprintf(pidCharArr, "%d", getpid());

    pid_t pid = fork();
    if (pid == 0) {
        char *const av[] = {"./sender", pidCharArr, amountOfSignals, mode, NULL};
        execvp("./sender", av);
        exit(0);
    }

    struct sigaction action;
    action.sa_sigaction = signalHandler;
    action.sa_flags = SA_SIGINFO;

    while (flagStopped == 0) {
        sigaction(SIGUSR1, &action, NULL);
        sigaction(SIGUSR2, &action, NULL);
    }
    printf("Wyslanych sygnałów: %s\n", amountOfSignals);
    printf("Odebranych przez catcher: %d\n\n", counter);
    printf("Wyslanych sygnałów: %d\n", counter);

    for (int i = 0; i < counter; i++)
        sendSignal(senderPid, SIGUSR1, mode);

    sendSignal(senderPid, SIGUSR2, mode);
}