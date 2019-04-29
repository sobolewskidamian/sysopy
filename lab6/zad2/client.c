#include <signal.h>
#include <sys/types.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include "chat.h"

char *queue_name;
mqd_t server_queue_descriptor;
mqd_t client_queue_descriptor;
int client_id;
struct Message message;

void readInstructions(FILE *);

void sendRequest();

void ServerMessages();

void sendEcho(const char *text) {
    message.req_type = ECHO;
    message.num1 = client_id;
    strcpy(message.arg1, text);
    sendRequest();
}

void sendToAll(const char *text) {
    message.req_type = ECHO_TO_ALL;
    message.num1 = client_id;
    strcpy(message.arg1, text);
    sendRequest();
}

void sendToFriends(const char *text) {
    message.req_type = ECHO_TO_FRIENDS;
    message.num1 = client_id;
    strcpy(message.arg1, text);
    sendRequest();
}

void sendToOne(int target_id, const char *text) {
    message.req_type = ECHO_TO_PERSON;
    message.num1 = client_id;
    message.num2 = target_id;
    strcpy(message.arg1, text);
    sendRequest();
}

void sendList() {
    message.req_type = LIST;
    message.num1 = client_id;
    sendRequest();
}

void sendStop() {
    message.req_type = STOP;
    message.num1 = client_id;
    message.num2 = client_queue_descriptor;
    sendRequest();
}

void sendFriends(const char *text) {
    message.req_type = FRIENDS;
    message.num1 = client_id;
    strcpy(message.arg1, text);
    sendRequest();
}

void ServerShutdown() {
    mq_close(server_queue_descriptor);
    mq_close(client_queue_descriptor);
    mq_unlink(queue_name);
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
        while (1) {
            if (getline(&buffor, &size, f) == -1)
                break;
            execInstruction(buffor);
        }
    }

    free(buffor);
}


void sendRequest() {
    mq_send(server_queue_descriptor, (char *) &message, sizeof(struct Message), message.req_type);
}

void sendInit() {
    message.req_type = INIT;
    strcpy(message.arg1, queue_name);
    sendRequest();
    mq_receive(client_queue_descriptor, (char *) &message, sizeof(struct Message), NULL);
    printf("Client ID: %d\n", message.num1);
    client_id = message.num1;
}

void ServerMessages() {
    if (mq_receive(client_queue_descriptor, (char *) &message, sizeof(struct Message), NULL) < 0) {
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

char *queueName() {
    char *name = calloc(32, sizeof(char));
    strcat(name, "/");
    sprintf(name + 1, "%d", getpid());
    return name;
}


int main(int argc, char **argv) {
    signal(SIGINT, handleExit);

    server_queue_descriptor = mq_open(SERVER_NAME, O_WRONLY);

    if (server_queue_descriptor < 0) {
        printf("Error while connecting to server\n");
        return 1;
    }

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct Message);
    attr.mq_curmsgs = 0;

    queue_name = queueName();
    mq_unlink(queue_name);
    client_queue_descriptor = mq_open(queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);

    if (client_queue_descriptor < 0) {
        printf("Error while opening server\n");
        return 1;
    }

    printf("Server Queue Descriptor: %d\n", server_queue_descriptor);
    printf("Private Queue Descriptor: %d\n", client_queue_descriptor);

    sendInit();
    readInstructions(stdin);
}
