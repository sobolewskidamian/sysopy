//
// Created by damian on 27.03.19.
//

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int flagStopped = 0;
int counter = 0;

void signalHandler(int sig) {
    if (sig == SIGUSR1) {
        counter++;
    } else if (sig == SIGUSR2) {
        flagStopped = 1;
    }
}

void sendSignal(int pid, int flag, char *mode) {
    if (strcmp(mode, "KILL") == 0)
        kill(pid, flag);
}

int main(int argc, char **argv) {
    int pid = (int) strtol(argv[1], (char **) NULL, 10);
    int amountOfSignals = (int) strtol(argv[2], (char **) NULL, 10);
    char *mode = argv[3];
    printf("Sender PID: %d\n\n", getpid());

    for (int i = 0; i < amountOfSignals; i++)
        sendSignal(pid, SIGUSR1, mode);

    sendSignal(pid, SIGUSR2, mode);

    while (flagStopped == 0) {
        signal(SIGUSR1, signalHandler);
        signal(SIGUSR2, signalHandler);
    }
    printf("Odebranych przez sender: %d\n", counter);
}