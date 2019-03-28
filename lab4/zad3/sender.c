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
int flagStopped = 0;
int counter = 0;
int amountOfSentByCatcher = 0;

void signalHandler(int sig, siginfo_t *info, void *vp) {
    if (sig == sig1) {
        counter++;
    } else if (sig == sig2) {
        flagStopped = 1;
        if (strcmp(mode, "SIGQUEUE") == 0) {
            union sigval sigval = info->si_value;
            amountOfSentByCatcher = sigval.sival_int;
        }
    }
}

void sendSignal(int pid, int flag, char *mode) {
    if (strcmp(mode, "KILL") == 0 || strcmp(mode, "SIGRT") == 0)
        kill(pid, flag);
    else if (strcmp(mode, "SIGQUEUE") == 0) {
        union sigval value;
        value.sival_int = counter;
        sigqueue(pid, flag, value);
    }
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
    int pid = (int) strtol(argv[1], (char **) NULL, 10);
    int amountOfSignals = (int) strtol(argv[2], (char **) NULL, 10);
    mode = argv[3];
    initSignals();

    printf("Sender PID: %d\n\n", getpid());

    for (int i = 0; i < amountOfSignals; i++) {
        sendSignal(pid, sig1, mode);
    }

    sendSignal(pid, sig2, mode);

    struct sigaction action;
    action.sa_sigaction = signalHandler;
    action.sa_flags = SA_SIGINFO;

    while (flagStopped == 0) {
        sigaction(sig1, &action, NULL);
        sigaction(sig2, &action, NULL);
    }
    printf("Sender:\n");
    if (strcmp(mode, "SIGQUEUE") == 0)
        printf("Wysłanych przez catcher: %d\n", amountOfSentByCatcher);
    printf("Odebranych przez sender: %d\n", counter);
}