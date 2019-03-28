//
// Created by damian on 27.03.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int sig1, sig2;
int flagStopped = 0;
int counter = 0;
int senderPid = 0;
char *mode;

void signalHandler(int sig, siginfo_t *info, void *vp) {
    if (sig == sig1) {
        counter++;
    } else if (sig == sig2) {
        flagStopped = 1;
    }
    senderPid = info->si_pid;
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

void sendSignal(int pid, int flag, char *mode) {
    if (strcmp(mode, "KILL") == 0 || strcmp(mode, "SIGRT") == 0) {
        kill(pid, flag);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        union sigval value;
        value.sival_int = counter;
        sigqueue(pid, flag, value);
    }
}

int main(int argc, char **argv) {
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);
    sigdelset(&sigset, SIGUSR2);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    if (argc < 3) {
        printf("Bad arguments\n");
        return 1;
    } else if (strcmp(argv[2], "KILL") != 0 && strcmp(argv[2], "SIGQUEUE") != 0 && strcmp(argv[2], "SIGRT") != 0) {
        printf("Bad mode - KILL, SIGQUEUE, SIGRT\n");
        return 1;
    }
    if (strcmp(argv[2], "SIGRT") == 0) {
        sigset_t sigset2;
        sigfillset(&sigset2);
        sigdelset(&sigset2, SIGRTMIN);
        sigdelset(&sigset2, SIGRTMIN + 1);
        sigprocmask(SIG_BLOCK, &sigset2, NULL);
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    }

    char *amountOfSignals = argv[1];
    mode = argv[2];
    initSignals();

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
        sigaction(sig1, &action, NULL);
        sigaction(sig2, &action, NULL);
    }

    printf("Catcher:\n");
    printf("Wyslanych sygnałów: %s\n", amountOfSignals);
    printf("Odebranych przez catcher = wyslanych do sendera: %d\n\n", counter);

    for (int i = 0; i < counter; i++)
        sendSignal(senderPid, sig1, mode);

    sendSignal(senderPid, sig2, mode);
}