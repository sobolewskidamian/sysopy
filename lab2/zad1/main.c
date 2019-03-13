#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include "library.h"

double calculateTime(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++)
        argv[i - 1] = argv[i];
    argc--;

    if (argc < 1) {
        printf("There's no arguments");
        return 0;
    }

    struct tms* startTimeTms = malloc(sizeof(struct tms));
    struct tms* endTimeTms = malloc(sizeof(struct tms));
    clock_t startTimeUsr;
    clock_t endTimeUsr;
    startTimeUsr = times(startTimeTms);

    if (strcmp(argv[0], "generate") == 0 && argc >= 4) {
        int blockSize = (int) strtol(argv[3], NULL, 10);
        int amountOfBlocks = (int) strtol(argv[2], NULL, 10);
        generate(blockSize, amountOfBlocks, argv[1]);

    } else if (strcmp(argv[0], "sort") == 0 && argc >= 4) {
        int blockSize = (int) strtol(argv[3], NULL, 10);
        int amountOfBlocks = (int) strtol(argv[2], NULL, 10);

        if (argc >= 5 && strcmp(argv[4], "sys") == 0)
            sort_sys(blockSize, amountOfBlocks, argv[1]);
        else
            sort_lib(blockSize, amountOfBlocks, argv[1]);

        endTimeUsr = times(endTimeTms);
        printf("%s %d %d %s\n", "sort", amountOfBlocks, blockSize, argv[4]);
        printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
        printf("%lf\t", calculateTime(startTimeUsr, endTimeUsr));
        printf("%lf\t", calculateTime(startTimeTms->tms_utime, endTimeTms->tms_utime));
        printf("%lf\n\n\n", calculateTime(startTimeTms->tms_stime, endTimeTms->tms_stime));

    } else if (strcmp(argv[0], "copy") == 0 && argc >= 5) {
        int blockSize = (int) strtol(argv[4], NULL, 10);
        int amountOfBlocks = (int) strtol(argv[3], NULL, 10);

        if (argc >= 6 && strcmp(argv[5], "sys") == 0)
            copy_sys(blockSize, amountOfBlocks, argv[1], argv[2]);
        else
            copy_lib(blockSize, amountOfBlocks, argv[1], argv[2]);

        endTimeUsr = times(endTimeTms);
        printf("%s %d %d %s\n", "copy", amountOfBlocks, blockSize, argv[5]);
        printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
        printf("%lf\t", calculateTime(startTimeUsr, endTimeUsr));
        printf("%lf\t", calculateTime(startTimeTms->tms_utime, endTimeTms->tms_utime));
        printf("%lf\n\n\n", calculateTime(startTimeTms->tms_stime, endTimeTms->tms_stime));
    } else {
        printf("Bad arguments");
        return 0;
    }

    free(startTimeTms);
    free(endTimeTms);

    /*generate(8192,1500, "temp1.txt");
    copy_sys(8192, 1500, "temp1.txt", "temp2.txt");
    sort_sys(8192,1500, "temp1.txt");*/
    return 0;
}