//
// Created by Damian on 07.03.2019.
//

#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

struct arr *createArray(int numberOfBlocks) {
    if (numberOfBlocks < 0)
        return NULL;

    struct arr *result = malloc(sizeof(struct arr));
    result->numberOfBlocks = numberOfBlocks;
    char **block = calloc(numberOfBlocks, sizeof(char *));
    result->blocks = block;
    return result;
}

int addBlock(struct arr *array, int size, char *block) {
    if (array == NULL)
        return -1;

    int index = 0;
    while (array->blocks[index] != NULL && index < array->numberOfBlocks)
        index++;

    if (index < array->numberOfBlocks) {
        array->blocks[index] = calloc(size, sizeof(char));
        strcpy(array->blocks[index], block);
        free(block);
        return index;
    } else
        return -1;
}

void removeBlock(struct arr *array, int index) {
    if (array == NULL || array->numberOfBlocks <= index || array->blocks[index] == NULL)
        return;

    free(array->blocks[index]);
    array->blocks[index] = NULL;
}

void removeArray(struct arr *array) {
    if (array == NULL)
        return;

    for (int i = 0; i < array->numberOfBlocks; i++) {
        if (array->blocks[i] != NULL) {
            free(array->blocks[i]);
            array->blocks[i] = NULL;
        }
    }
}

int isDirectory(const char *path) {
    struct stat statbuf;
    stat(path, &statbuf);
    return S_ISDIR(statbuf.st_mode);
}

int find(char *directory, const char *name, char *temporaryFile) {
    DIR *d = opendir(directory);
    if (d)
        closedir(d);
    else return -1;

    int commandLength;
    char *commandBuffer;

    commandLength = strlen(directory) + strlen(name) + strlen(temporaryFile) + 50;
    commandBuffer = calloc(commandLength, sizeof(char));
    sprintf(commandBuffer, "find %s -name %s > %s", directory, name, temporaryFile);
    printf("%s\n", commandBuffer);

    int res = system(commandBuffer);
    free(commandBuffer);

    return res;
}

int getSizeOfFile(FILE *fp) {
    if (fp == NULL)
        return -1;

    fseek(fp, 0L, SEEK_END);
    long int result = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return result;
}

int addTemporaryFileToBlock(struct arr *array, char *temporaryFile) {
    FILE *fp = fopen(temporaryFile, "r");

    int size = getSizeOfFile(fp);
    char *result = calloc(size, sizeof(char));

    char ch;
    int i = 0;

    while ((ch = fgetc(fp)) != EOF)
        result[i++] = ch;

    int index = addBlock(array, size, result);

    fclose(fp);
    return index;
}

void searchDirectory(char *dir, char *fileName, char *tempFile) {
    remove(tempFile);
    find(dir, fileName, tempFile);
}