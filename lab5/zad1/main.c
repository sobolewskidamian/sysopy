#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define maxNumberOfCharsInLine 256
#define maxNumberOfCommands 100

char **parseArguments(char *line) {
    int size = 0;
    char **args = NULL;
    char whiteSigns[3] = {'\n', '\t', ' '};
    char *arg = strtok(line, whiteSigns);
    while (arg != NULL) {
        size++;
        args = realloc(args, sizeof(char *) * size);
        if (args == NULL)
            exit(1);

        args[size - 1] = arg;
        arg = strtok(NULL, whiteSigns);
    }
    args = realloc(args, sizeof(char *) * (size + 1));
    if (args == NULL)
        exit(1);

    args[size] = NULL;
    return args;
}


void executeLine(char registry[]) {
    int commandCount = 0;
    int pipes[2][2];
    char *commands[maxNumberOfCommands];

    while ((commands[commandCount] = strtok(commandCount == 0 ? registry : NULL, "|")) != NULL)
        commandCount++;

    int i;
    for (i = 0; i < commandCount; i++) {
        if (i > 0) {
            close(pipes[i % 2][0]);
            close(pipes[i % 2][1]);
        }

        pid_t pid = fork();
        if (pid == 0) {
            char **exec = parseArguments(commands[i]);

            if (i != commandCount - 1) {
                close(pipes[i % 2][0]);
                dup2(pipes[i % 2][1], STDOUT_FILENO);
            }
            if (i != 0) {
                close(pipes[(i + 1) % 2][1]);
                dup2(pipes[(i + 1) % 2][0], STDIN_FILENO);
            }
            execvp(exec[0], exec);
            exit(0);
        }
    }
    close(pipes[i % 2][0]);
    close(pipes[i % 2][1]);
    wait(NULL);
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Bad arguments\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Can't open given file");
        return 1;
    }

    char registry[maxNumberOfCharsInLine];
    int index = 0;

    while (fgets(registry, maxNumberOfCharsInLine, file))
        index++;
    fseek(file, 0, SEEK_SET);

    for (int i = 0; i < index; i++) {
        fgets(registry, maxNumberOfCharsInLine, file);
        pid_t pid = fork();
        if (pid == 0) {
            executeLine(registry);
            exit(0);
        }
        int statLock = 0;
        wait(&statLock);
    }
    fclose(file);

    return 0;
}