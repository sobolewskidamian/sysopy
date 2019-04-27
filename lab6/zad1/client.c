#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include "chat.h"

int server_qid;
int client_qid;
int client_id;
pid_t readInstructionPid;
struct Message message;

void readInstructions(FILE *);

void sendRequest(int);

void ServerMessages();

void sendEcho(const char *text) {
    message.type = message.req_type = ECHO;
    message.num1 = client_id;
    strcpy(message.arg1, text);
    sendRequest(1);
}

void sendToAll(const char *text) {
    message.type = message.req_type = ECHO_TO_ALL;
    message.num1 = client_id;
    strcpy(message.arg1, text);
    sendRequest(1);
}

void sendToFriends(const char *text) {
    message.type = message.req_type = ECHO_TO_FRIENDS;
    message.num1 = client_id;
    strcpy(message.arg1, text);
    sendRequest(1);
}

void sendToOne(int target_id, const char *text) {
    message.type = message.req_type = ECHO_TO_PERSON;
    message.num1 = client_id;
    message.num2 = target_id;
    strcpy(message.arg1, text);
    sendRequest(1);
}

void sendList() {
    message.type = message.req_type = LIST;
    message.num1 = client_id;
    sendRequest(1);
}

void sendStop() {
    message.type = message.req_type = STOP;
    message.num1 = client_id;
    message.num2 = client_qid;
    sendRequest(1);
}

void sendFriends(const char *text) {
    message.type = message.req_type = FRIENDS;
    message.num1 = client_id;
    strcpy(message.arg1, text);
    sendRequest(1);
}

void ServerShutdown() {
    msgctl(client_qid, IPC_RMID, NULL);
    kill(readInstructionPid, SIGKILL);
    exit(1);
}

void execInstruction(char *instruction) {
    if (strncmp(instruction, "ECHO ", 5) == 0) {
        instruction += 5;
        sendEcho(instruction);
    } else if (strncmp(instruction, "2ALL ", 5) == 0) {
        instruction += 5;
        sendToAll(instruction);
    } else if (strncmp(instruction, "2FRIENDS ", 9) == 0) {
        instruction += 9;
        sendToFriends(instruction);
    } else if (strncmp(instruction, "2ONE ", 5) == 0) {
        instruction += 5;
        char *token = strtok(instruction, " ");
        int target = atoi(token);
        token = strtok(NULL, "");
        sendToOne(target, token);
    } else if (strncmp(instruction, "LIST", 4) == 0) {
        sendList();
    } else if (strncmp(instruction, "STOP", 4) == 0) {
        sendStop();
        ServerShutdown();
    } else if (strncmp(instruction, "FRIENDS ", 8) == 0) {
        instruction += 8;
        sendFriends(instruction);
    } else if (strncmp(instruction, "READ ", 5) == 0) {
        instruction += 5;

        char *path = calloc(256, sizeof(char));
        strcpy(path, instruction);
        path[strcspn(path, "\n")] = 0;

        FILE *f = fopen(path, "r");
        if (f == NULL) {
            printf("Unable to open\n");
            free(path);
            return;
        }

        kill(readInstructionPid, SIGKILL);
        readInstructions(f);
        fclose(f);
        free(path);
    } else if (strcmp(instruction, "\n") == 0) {

    } else {
        printf("Bad command\n");
    }
}


void readInstructions(FILE *f) {
    char *buffor = calloc(MAX_MESSAGE_SIZE, sizeof(char));
    size_t size;

    pid_t pid = fork();
    if (pid == 0) {
        while (1)
            ServerMessages();
    } else {
        readInstructionPid = pid;
        while (1) {
            if (getline(&buffor, &size, f) == -1)
                break;
            execInstruction(buffor);
        }
    }

    free(buffor);
}


void sendRequest(int waitFlag) {
    msgsnd(server_qid, &message, sizeof(struct Message) - sizeof(long), 0);

    if (waitFlag == 0) {
        struct Message res;
        msgrcv(client_qid, &res, sizeof(struct Message) - sizeof(long), 0, 0);

        message.num1 = res.num1;
        message.num2 = res.num2;
        strcpy(message.arg1, res.arg1);
        strcpy(message.arg2, res.arg2);
    }
}

void sendInit() {
    message.type = message.req_type = INIT;
    message.num1 = client_qid;
    sendRequest(0);
    printf("Client ID: %d\n", message.num1);
    client_id = message.num1;
}

void ServerMessages() {
    if (msgrcv(client_qid, &message, sizeof(struct Message) - sizeof(long), BY_PRIORITY, 0) == -1) {
        return;
    }

    switch (message.req_type) {
        case STOP:
            ServerShutdown();
            break;

        case ECHO:
            printf("%s\n", message.arg1);
            break;

        case LIST:
            printf("%s\n", message.arg1);
            printf("%s\n", message.arg2);
            break;

        default:
            break;
    }
}

void handleExit() {
    sendStop();
    ServerShutdown();
}

int main(int argc, char **argv) {
    signal(SIGINT, handleExit);

    key_t server_key = ftok(SERVER_KEY_PATH, SERVER_KEY_SEED);

    if ((server_qid = msgget(server_key, QUEUE_PERMISSIONS)) == -1) {
        printf("Error while connecting to server\n");
        return 1;
    }

    if ((client_qid = msgget(IPC_PRIVATE, QUEUE_PERMISSIONS)) == -1) {
        printf("Error while creating client queue\n");
        return 1;
    }

    printf("Server Queue ID: %d\n", server_qid);
    printf("Private Queue ID: %d\n", client_qid);

    sendInit();
    readInstructions(stdin);
}
