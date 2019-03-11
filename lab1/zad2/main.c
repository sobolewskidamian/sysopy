#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <memory.h>
#include "library.h"

double calculateTime(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++)
        argv[i - 1] = argv[i];
    argc--;

    int arrTimeCount = argc + 7;
    struct tms **tmsTime = malloc(arrTimeCount * sizeof(struct tms *));
    clock_t realTime[arrTimeCount];
    for (int i = 0; i < arrTimeCount; i++) {
        tmsTime[i] = (struct tms *) malloc(sizeof(struct tms *));
    }
    char **tempFiles = NULL;
    int tempFileIterator = 0;
    struct arr *array = NULL;

    int index = 0, iterator = 0, arrSize = 0, timesIterator = 0;
    while (index < argc) {
        if (index == 0 && strcmp(argv[0], "create_table") != 0) {
            printf("Bad arguments - table wasn't created at first");
            return 0;
        }

        if (index == 0 && strcmp(argv[0], "create_table") == 0 && argc >= 2) {
            if (atoi(argv[1]) <= 0) {
                printf("Bad arguments - table can't be created");
                return 0;
            }

            arrSize = strtol(argv[1], NULL, 10);
            index += 2;
            array = createArray(arrSize);
            tempFiles = calloc(arrSize, sizeof(char *));

            realTime[0] = times(tmsTime[0]);
            timesIterator++;
        } else if (index >= 2 && strcmp(argv[index], "remove_block") == 0 && argc >= index + 2) {
            int ind = strtol(argv[index + 1], NULL, 10);
            index += 2;
            removeBlock(array, ind);

            realTime[timesIterator] = times(tmsTime[timesIterator]);
            printf("%s %d \n", "remove_block", ind);
            printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
            printf("%lf\t", calculateTime(realTime[timesIterator - 1], realTime[timesIterator]));
            printf("%lf\t", calculateTime(tmsTime[timesIterator - 1]->tms_utime, tmsTime[timesIterator]->tms_utime));
            printf("%lf\n\n\n",
                   calculateTime(tmsTime[timesIterator - 1]->tms_stime, tmsTime[timesIterator]->tms_stime));
            timesIterator++;
        } else if (index >= 2 && strcmp(argv[index], "search_directory") == 0 && argc >= index + 4) {
            char *dir = argv[index + 1];
            char *file = argv[index + 2];
            char *name_file_temp = argv[index + 3];
            index += 4;
            searchDirectory(dir, file, name_file_temp);
            tempFiles[tempFileIterator++] = name_file_temp;

            realTime[timesIterator] = times(tmsTime[timesIterator]);
            printf("%s %s %s %s\n", "search_directory", dir, file, name_file_temp);
            printf("%s\n", "search");
            printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
            printf("%lf\t", calculateTime(realTime[timesIterator - 1], realTime[timesIterator]));
            printf("%lf\t", calculateTime(tmsTime[timesIterator - 1]->tms_utime, tmsTime[timesIterator]->tms_utime));
            printf("%lf\n\n",
                   calculateTime(tmsTime[timesIterator - 1]->tms_stime, tmsTime[timesIterator]->tms_stime));
            timesIterator++;

            int a = addTemporaryFileToBlock(array, name_file_temp);
            realTime[timesIterator] = times(tmsTime[timesIterator]);
            printf("%s [%d]\n", "add", a);
            printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
            printf("%lf\t", calculateTime(realTime[timesIterator - 1], realTime[timesIterator]));
            printf("%lf\t", calculateTime(tmsTime[timesIterator - 1]->tms_utime, tmsTime[timesIterator]->tms_utime));
            printf("%lf\n\n\n",
                   calculateTime(tmsTime[timesIterator - 1]->tms_stime, tmsTime[timesIterator]->tms_stime));
            timesIterator++;;
        } else {
            printf("Bad arguments");
            return 0;
        }
        iterator++;
    }

    int numberOfOperationsAddAndRemove = 5;

    for (int x = 0; x < numberOfOperationsAddAndRemove; x++) {
        for (int i = 0; i < tempFileIterator && tempFiles != NULL; i++) {
            int a = addTemporaryFileToBlock(array, tempFiles[i]);
            removeBlock(array, a);
        }

        realTime[timesIterator] = times(tmsTime[timesIterator]);
        printf("%s %d\n", "add_and_remove", x);
        printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
        printf("%lf\t", calculateTime(realTime[timesIterator - 1], realTime[timesIterator]));
        printf("%lf\t", calculateTime(tmsTime[timesIterator - 1]->tms_utime, tmsTime[timesIterator]->tms_utime));
        printf("%lf\n\n",
               calculateTime(tmsTime[timesIterator - 1]->tms_stime, tmsTime[timesIterator]->tms_stime));
        timesIterator++;
    }

    removeArray(array);

    return 0;
}