//
// Created by damian on 12.03.19.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

void generate(int blockSize, int amountOfBlocks, char *path) {
    remove(path);

    for (int i = 0; i < amountOfBlocks; i++) {
        char *commandBuffer = calloc(100 + strlen(path), sizeof(char));
        char *commandBuffer2 = calloc(20 + strlen(path), sizeof(char));
        sprintf(commandBuffer, "cat /dev/urandom | base64 | head -c %d >> %s", blockSize, path);
        sprintf(commandBuffer2, "echo '' >> %s", path);

        system(commandBuffer);
        system(commandBuffer2);

        free(commandBuffer);
        free(commandBuffer2);
    }
}

void copy_lib(int blockSize, int amountOfBlocks, char *path1, char *path2) {
    remove(path2);

    FILE *fp1 = fopen(path1, "r");
    FILE *fp2 = fopen(path2, "w+");
    char *buffer = calloc((size_t) blockSize, sizeof(char));

    for (int i = 0; i < amountOfBlocks; i++) {
        fread(buffer, sizeof(char), (size_t) blockSize + 1, fp1);
        fwrite(buffer, sizeof(char), (size_t) blockSize + 1, fp2);
    }

    free(buffer);
    fclose(fp1);
    fclose(fp2);
}

void copy_sys(int blockSize, int amountOfBlocks, char *path1, char *path2) {
    remove(path2);

    int fp1 = open(path1, O_RDONLY);
    int fp2 = open(path2, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    char *buffer = calloc((size_t) blockSize, sizeof(char));

    for (int i = 0; i < amountOfBlocks; i++) {
        read(fp1, buffer, (size_t) (blockSize + 1) * sizeof(char));
        write(fp2, buffer, (size_t) (blockSize + 1) * sizeof(char));
    }

    free(buffer);
    close(fp1);
    close(fp2);
}

void sort_sys(int blockSize, int amountOfBlocks, char *path) {
    int file = open(path, O_RDWR);
    char *reg1 = calloc((size_t) blockSize + 1, sizeof(char));
    char *reg2 = calloc((size_t) blockSize + 1, sizeof(char));
    long int sizeOfBlockChar = (blockSize + 1) * sizeof(char);

    for (int i = 0; i < amountOfBlocks; i++) {
        lseek(file, i * sizeOfBlockChar, 0);
        read(file, reg1, (size_t) (blockSize + 1) * sizeof(char));

        int atIndex = -1;
        char charAtIndex = reg1[0];
        for (int j = i + 1; j < amountOfBlocks; j++) {
            lseek(file, j * sizeOfBlockChar, 0);
            read(file, reg2, (size_t) (blockSize + 1) * sizeof(char));

            if (reg2[0] < charAtIndex) {
                atIndex = j;
                charAtIndex = reg2[0];
            }

        }
        if (atIndex != -1) {
            lseek(file, atIndex * sizeOfBlockChar, 0);
            read(file, reg2, (size_t) (blockSize + 1) * sizeof(char));

            lseek(file, i * sizeOfBlockChar, 0);
            write(file, reg2, (size_t) (blockSize + 1) * sizeof(char));

            lseek(file, atIndex * sizeOfBlockChar, 0);
            write(file, reg1, (size_t) (blockSize + 1) * sizeof(char));
        }
    }
    close(file);
    free(reg1);
    free(reg2);
}

void sort_lib(int blockSize, int amountOfBlocks, char *path) {
    FILE *file = fopen(path, "r+");
    char *reg1 = calloc((size_t) blockSize + 1, sizeof(char));
    char *reg2 = calloc((size_t) blockSize + 1, sizeof(char));
    long int sizeOfBlockChar = (blockSize + 1) * sizeof(char);

    for (int i = 0; i < amountOfBlocks; i++) {
        fseek(file, i * sizeOfBlockChar, 0);
        fread(reg1, sizeof(char), (size_t) blockSize + 1, file);

        int atIndex = -1;
        char charAtIndex = reg1[0];
        for (int j = i + 1; j < amountOfBlocks; j++) {
            fseek(file, j * sizeOfBlockChar, 0);
            fread(reg2, sizeof(char), (size_t) blockSize + 1, file);

            if (reg2[0] < charAtIndex) {
                atIndex = j;
                charAtIndex = reg2[0];
            }

        }
        if (atIndex != -1) {
            fseek(file, atIndex * sizeOfBlockChar, 0);
            fread(reg2, sizeof(char), (size_t) blockSize + 1, file);

            fseek(file, i * sizeOfBlockChar, 0);
            fwrite(reg2, sizeof(char), (size_t) blockSize + 1, file);

            fseek(file, atIndex * sizeOfBlockChar, 0);
            fwrite(reg1, sizeof(char), (size_t) blockSize + 1, file);
        }
    }
}