//
// Created by Damian on 07.03.2019.
//

#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <dirent.h>

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

bool isDirectory(char *path) {
    if (strchr(path, '.'))
        return false;
    else return true;
}

void find(const char *directory, const char *name, char *temporaryFile) {
    DIR *d = opendir(directory);
    struct dirent *dir;

    if (d != NULL) {
        while ((dir = readdir(d)) != NULL) {
            char *entryName = dir->d_name;
            char *next = malloc(strlen(directory) + strlen(entryName) + 2);
            sprintf(next, "%s/%s", directory, entryName);
            if (isDirectory(entryName)) {
                if (strcmp(entryName, ".") == 0 || strcmp(entryName, "..") == 0)
                    continue;
                find(next, name, temporaryFile);
            } else {
                if (strcmp(entryName, name) == 0) {  //same strings
                    //*result = &*getPath(directory, name);
                    FILE *fp = fopen(temporaryFile, "a");
                    fprintf(fp, "%s\n", next);
                    fclose(fp);
                }
            }
        }
        free(d);
        closedir(d);
    }
}

void clenFile(char *file) {
    FILE *fp = fopen(file, "w");
    fclose(fp);
}

int getSizeOfFile(char *file) {
    FILE *fp = fopen(file, "r");
    if (fp == NULL)
        return -1;

    fseek(fp, 0L, SEEK_END);
    long int result = ftell(fp);
    fclose(fp);
    return result;
}

int addTemporaryFileToBlock(struct arr *array, char *temporaryFile) {
    int size = getSizeOfFile(temporaryFile);
    char *result = calloc(size, sizeof(char));

    FILE *fp = fopen(temporaryFile, "r");
    char ch;
    int i = 0;

    while ((ch = fgetc(fp)) != EOF)
        result[i++] = ch;

    int index = addBlock(array, size, result);
    fclose(fp);
    free(fp);
    return index;
}

int searchDirectoryAndAdd(struct arr *array, char *dir, char *fileName, char *tempFile) {
    clenFile(tempFile);
    find(dir, fileName, tempFile);
    return addTemporaryFileToBlock(array, tempFile);
}