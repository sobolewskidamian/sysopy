//
// Created by Damian on 07.03.2019.
//

#ifndef LAB1_LIBRARY_H
#define LAB1_LIBRARY_H

#include <stdbool.h>

struct arr {
    int numberOfBlocks;
    char **blocks;
};

struct arr *createArray(int);

void searchDirectory(char *, char *, char *);

int addTemporaryFileToBlock(struct arr *, char *);

void removeBlock(struct arr *, int);

void removeArray(struct arr *);

#endif //LAB1_LIBRARY_H