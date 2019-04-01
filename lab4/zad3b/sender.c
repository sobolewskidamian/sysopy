//
// Created by damian on 27.03.19.
//

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int sig1, sig2;
char *mode;
int catcherPid;
int counter = 0;
int amountOfSignals;
int responded = 0;


void sendSignal(int, int, char *);

void signalHandler(int sig, siginfo_t *info, void *vp) {
    if (sig == sig1) {
        counter++;
        responded = 1;
    }
}

void sendSignal(int pid, int flag, char *mode) {
    int pom = amountOfSignals;
    while (pom--) {
        responded = 0;
        if (strcmp(mode, "KILL") == 0 || strcmp(mode, "SIGRT") == 0)
            kill(pid, flag);
        else if (strcmp(mode, "SIGQUEUE") == 0) {
            union sigval value;
            value.sival_int = counter;
            sigqueue(pid, flag, value);
        }
        printf("Sended %s to %d. Total: %d\n", strsignal(sig1), pid, counter);
        while (responded != 1) {}
    }
    kill(catcherPid, sig2);
}

void initSignals() {
    if (strcmp(mode, "SIGRT") == 0) {
        sig1 = SIGRTMIN;
        sig2 = SIGRTMIN + 1;
    } else {
        sig1 = SIGUSR1;
        sig2 = SIGUSR2;
    }
}

int main(int argc, char **argv) {
    catcherPid = (int) strtol(argv[1], (char **) NULL, 10);
    amountOfSignals = (int) strtol(argv[2], (char **) NULL, 10);
    mode = argv[3];
    initSignals();

    printf("Sender PID: %d\n\n", getpid());

    struct sigaction action;
    action.sa_sigaction = signalHandler;
    action.sa_flags = SA_SIGINFO;
    sigaction(sig1, &action, NULL);
    sigaction(sig2, &action, NULL);

    sendSignal(catcherPid, sig1, mode);
}