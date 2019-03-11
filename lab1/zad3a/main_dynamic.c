#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <memory.h>
#include <dlfcn.h>

void *handle;

struct arr {
    int numberOfBlocks;
    char **blocks;
};

double calculateTime(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

int main(int argc, char *argv[]) {
    handle = dlopen("./library.so", RTLD_LAZY);
    if (!handle) {
        printf("%s\n", dlerror());
        return 0;
    }


    for (int i = 1; i < argc; i++)
        argv[i - 1] = argv[i];
    argc--;
    int arrTimeCount = 20;
    struct tms **tms_time = malloc(arrTimeCount * sizeof(struct tms *));
    clock_t real_time[arrTimeCount];
    for (int i = 0; i < arrTimeCount; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms *));
    }
    char **tempFiles = NULL;
    int tempFileIterator = 0;
    struct arr *array = NULL;

    int index = 0, iterator = 0, arrSize = -1, timesIterator = 0;
    while (index < argc /*&& (arrSize == -1 || iterator <= arrSize)*/) {
        if (index == 0 && strcmp(argv[0], "create_table") == 0 && argc >= 2) {
            arrSize = strtol(argv[1], NULL, 10);
            index += 2;

            struct arr *(*createArray)(int) = dlsym(handle, "createArray");
            array = createArray(arrSize);
            tempFiles = calloc(arrSize, sizeof(char *));

            real_time[0] = times(tms_time[0]);
            timesIterator++;
        } else if (index >= 2 && strcmp(argv[index], "remove_block") == 0 && argc >= index + 2) {
            int ind = strtol(argv[index + 1], NULL, 10);
            index += 2;

            void (*removeBlock)(struct arr *, int) = dlsym(handle, "removeBlock");
            removeBlock(array, ind);

            real_time[timesIterator] = times(tms_time[timesIterator]);
            printf("%s %d \n", "remove_block", ind);
            printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
            printf("%lf\t", calculateTime(real_time[timesIterator - 1], real_time[timesIterator]));
            printf("%lf\t", calculateTime(tms_time[timesIterator - 1]->tms_utime, tms_time[timesIterator]->tms_utime));
            printf("%lf\n\n\n",
                   calculateTime(tms_time[timesIterator - 1]->tms_stime, tms_time[timesIterator]->tms_stime));
            timesIterator++;
        } else if (index >= 2 && strcmp(argv[index], "search_directory") == 0 && argc >= index + 4) {
            char *dir = argv[index + 1];
            char *file = argv[index + 2];
            char *name_file_temp = argv[index + 3];
            index += 4;

            void (*searchDirectory)(char *, char *, char *) = dlsym(handle, "searchDirectory");
            searchDirectory(dir, file, name_file_temp);
            tempFiles[tempFileIterator++] = name_file_temp;

            real_time[timesIterator] = times(tms_time[timesIterator]);
            printf("%s %s %s %s\n", "search_directory", dir, file, name_file_temp);
            printf("%s\n", "search");
            printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
            printf("%lf\t", calculateTime(real_time[timesIterator - 1], real_time[timesIterator]));
            printf("%lf\t", calculateTime(tms_time[timesIterator - 1]->tms_utime, tms_time[timesIterator]->tms_utime));
            printf("%lf\n\n",
                   calculateTime(tms_time[timesIterator - 1]->tms_stime, tms_time[timesIterator]->tms_stime));
            timesIterator++;


            int (*addTemporaryFileToBlock)(struct arr *, char *) = dlsym(handle, "addTemporaryFileToBlock");
            int a = addTemporaryFileToBlock(array, name_file_temp);
            real_time[timesIterator] = times(tms_time[timesIterator]);
            printf("%s [%d]\n", "add", a);
            printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
            printf("%lf\t", calculateTime(real_time[timesIterator - 1], real_time[timesIterator]));
            printf("%lf\t", calculateTime(tms_time[timesIterator - 1]->tms_utime, tms_time[timesIterator]->tms_utime));
            printf("%lf\n\n\n",
                   calculateTime(tms_time[timesIterator - 1]->tms_stime, tms_time[timesIterator]->tms_stime));
            timesIterator++;;
        } else {
            printf("Bad arguments");
            return 0;
        }
        iterator++;
    }

    int numberOfOperationsAddAndRemove = 5;

    for (int x = 0; x < numberOfOperationsAddAndRemove; x++) {
        for (int i = 0; i < tempFileIterator; i++) {
            int (*addTemporaryFileToBlock)(struct arr *, char *) = dlsym(handle, "addTemporaryFileToBlock");
            int a = addTemporaryFileToBlock(array, tempFiles[i]);

            void (*removeBlock)(struct arr *, int) = dlsym(handle, "removeBlock");
            removeBlock(array, a);
        }

        real_time[timesIterator] = times(tms_time[timesIterator]);
        printf("%s %d\n", "add_and_remove", x);
        printf("%s\t\t%s\t\t%s\n", "Real", "User", "System");
        printf("%lf\t", calculateTime(real_time[timesIterator - 1], real_time[timesIterator]));
        printf("%lf\t", calculateTime(tms_time[timesIterator - 1]->tms_utime, tms_time[timesIterator]->tms_utime));
        printf("%lf\n\n",
               calculateTime(tms_time[timesIterator - 1]->tms_stime, tms_time[timesIterator]->tms_stime));
        timesIterator++;
    }

    void (*removeArray)(struct arr *) = dlsym(handle, "removeArray");
    removeArray(array);
    dlclose(handle);

    return 0;
}