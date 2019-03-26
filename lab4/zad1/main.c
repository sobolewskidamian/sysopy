#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int awaitingHandler = 0;

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
    time_t actTime;
    act.sa_handler = au;
    act.sa_flags = 0;

    while(1){
        sigaction(SIGTSTP, &act, NULL);
        signal(SIGINT, initSignal);

        if(awaitingHandler==0){
            char buf[50];
            actTime = time(NULL);
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&actTime));
            printf("%s\n", buf);
        }

        sleep(1);
    }
}