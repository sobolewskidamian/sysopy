#define _GNU_SOURCE

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "chat.h"

struct Message request;
mqd_t queue_descriptor;
mqd_t clients[MAX_ROOM_SIZE];
int friends[MAX_ROOM_SIZE][MAX_ROOM_SIZE];

void printRequest() {
    printf("MESSAGE TYPE: %d\n", request.req_type);
    printf("NUM1: %d\n", request.num1);
    printf("NUM2: %d\n", request.num2);
    printf("ARG1: %s\n", request.arg1);
    printf("ARG2: %s\n\n\n", request.arg2);
}

void timestampMessage(int uid, char *text) {
    char temp[MAX_MESSAGE_SIZE];
    strcpy(temp, text);

    time_t raw_time;
    time(&raw_time);

    char timestamp[50];
    strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S", localtime(&raw_time));

    sprintf(text, "[%s] | %d: %s", timestamp, uid, temp);
}

void removeActiveFriends(int client_id) {
    for (int i = 0; i < MAX_ROOM_SIZE; i++)
        friends[client_id][i] = -1;
}

void respondInit() {
    int i = 0;
    while (i < MAX_ROOM_SIZE && clients[i] != 0) i++;

    if (i == MAX_ROOM_SIZE)
        return;

    char *client_name = request.arg1;
    mqd_t client_qd = mq_open(client_name, O_WRONLY);
    clients[i] = client_qd;
    request.num1 = i;

    mq_send(client_qd, (const char *) &request, sizeof(struct Message), request.req_type);
}

void respondStop() {
    mqd_t client_qd = request.num2;
    request.req_type = STOP;
    for (int i = 0; i < MAX_ROOM_SIZE; i++)
        if (clients[i] == client_qd) clients[i] = 0;

    removeActiveFriends(client_qd);
    request.num1 = -1;
    request.num2 = -1;

    mq_send(client_qd, (const char *) &request, sizeof(struct Message), request.req_type);
    mq_close(client_qd);
}

void respondEcho() {
    mqd_t client_qd = clients[request.num1];
    request.req_type = ECHO;
    timestampMessage(request.num1, request.arg1);

    mq_send(client_qd, (const char *) &request, sizeof(struct Message), request.req_type);
}

void respondToAll() {
    request.req_type = ECHO;
    timestampMessage(request.num1, request.arg1);

    for (int i = 0; i < MAX_ROOM_SIZE; i++) {
        mqd_t client_qd = clients[i];
        if (client_qd == 0)
            continue;

        mq_send(client_qd, (const char *) &request, sizeof(struct Message), request.req_type);
    }
}

void respondToFriends() {
    request.req_type = ECHO;
    timestampMessage(request.num1, request.arg1);

    for (int i = 0; i < MAX_ROOM_SIZE && friends[request.num1][i] != -1; i++) {
        mqd_t client_qd = clients[friends[request.num1][i]];
        if (client_qd == 0)
            continue;

        mq_send(client_qd, (const char *) &request, sizeof(struct Message), request.req_type);
    }
}

void respondToOne() {
    request.req_type = ECHO;
    timestampMessage(request.num1, request.arg1);

    mqd_t client_qd = clients[request.num2];
    if (client_qd == 0)
        return;

    mq_send(client_qd, (const char *) &request, sizeof(struct Message), request.req_type);
}

void respondList() {
    mqd_t client_qd = clients[request.num1];
    request.req_type = LIST;

    memset(request.arg1, 0, sizeof(request.arg1));
    strcat(request.arg1, "Active Users:\n");

    for (int i = 0; i < MAX_ROOM_SIZE; i++) {
        if (clients[i] != 0) {
            char str[32];
            sprintf(str, "%d", i);
            strcat(request.arg1, str);
        }

        if (i == request.num1) strcat(request.arg1, " (You)");
        if (clients[i] != 0) strcat(request.arg1, "\n");
    }

    memset(request.arg2, 0, sizeof(request.arg2));
    strcat(request.arg2, "Current friends:\n");

    for (int i = 0; i < MAX_ROOM_SIZE && friends[request.num1][i] != -1; i++) {
        char str[32];
        sprintf(str, "%d", friends[request.num1][i]);
        strcat(request.arg2, str);
        strcat(request.arg2, "\n");
    }

    mq_send(client_qd, (const char *) &request, sizeof(struct Message), request.req_type);
}

void respondFriends() {
    mqd_t client_qd = clients[request.num1];

    char *token = strtok(request.arg1, " ");
    if (strcmp(token, "ADD") == 0) {
        token = strtok(NULL, " ");
        while (token != NULL) {
            int notRepeat = 1;
            for (int i = 0; i < MAX_ROOM_SIZE; i++)
                if (friends[request.num1][i] == atoi(token))
                    notRepeat = 0;

            if (notRepeat == 0) {
                token = strtok(NULL, " ");
                continue;
            }

            int id = atoi(token);
            for (int i = 0; id >= 0 && id < MAX_ROOM_SIZE && i < MAX_ROOM_SIZE; i++) {
                if (friends[request.num1][i] == -1) {
                    friends[request.num1][i] = id;
                    token = strtok(NULL, " ");
                    break;
                }
            }
        }
    } else if (strcmp(token, "DEL") == 0) {
        token = strtok(NULL, " ");

        while (token != NULL) {
            int id = atoi(token);
            for (int i = 0; id >= 0 && id < MAX_ROOM_SIZE && i < MAX_ROOM_SIZE; i++) {
                if (friends[request.num1][i] == id) {
                    friends[request.num1][i] = -1;
                    token = strtok(NULL, " ");
                    int iterator = i + 1;
                    while (friends[request.num1][iterator] != -1) {
                        friends[request.num1][iterator - 1] = friends[request.num1][iterator];
                        friends[request.num1][iterator] = -1;
                        iterator++;
                    }
                    break;
                }
            }
        }
    } else {
        for (int i = 0; i < MAX_ROOM_SIZE; i++)
            friends[request.num1][i] = -1;

        int i = 0;
        while (token != NULL && i < MAX_ROOM_SIZE) {
            int id = atoi(token);
            if (id >= 0 && id < MAX_ROOM_SIZE)
                friends[request.num1][i++] = id;
            token = strtok(NULL, " ");
        }
    }

    mq_send(client_qd, (const char *) &request, sizeof(struct Message), request.req_type);
}

void handleExit() {
    request.req_type = STOP;

    for (int i = 0; i < MAX_ROOM_SIZE; i++) {
        if (clients[i] == 0)
            continue;
        mq_send(clients[i], (const char *) &request, sizeof(struct Message), request.req_type);
        mq_close(clients[i]);
    }

    mq_close(queue_descriptor);
    mq_unlink(SERVER_NAME);
    exit(0);
}

int main(int argc, char **argv) {
    memset(clients, 0, sizeof(clients));
    for (int i = 0; i < MAX_ROOM_SIZE; i++)
        for (int j = 0; j < MAX_ROOM_SIZE; j++)
            friends[i][j] = -1;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct Message);
    attr.mq_curmsgs = 0;

    mq_unlink(SERVER_NAME);
    queue_descriptor = mq_open(SERVER_NAME, O_RDWR | O_CREAT, QUEUE_PERMISSIONS, &attr);

    signal(SIGINT, handleExit);

    printf("========Chat server=========\n");
    printf("Queue ID: %d\nListening...\n", queue_descriptor);

    while (1) {
        mq_receive(queue_descriptor, (char *) &request, sizeof(request), NULL);

        printRequest();

        switch (request.req_type) {
            case INIT:
                respondInit();
                break;
            case STOP:
                respondStop();
                break;
            case ECHO:
                respondEcho();
                break;
            case ECHO_TO_ALL:
                respondToAll();
                break;
            case ECHO_TO_FRIENDS:
                respondToFriends();
                break;
            case ECHO_TO_PERSON:
                respondToOne();
                break;
            case LIST:
                respondList();
                break;
            case FRIENDS:
                respondFriends();
                break;
            default:
                printf("Bad request\n");
                break;
        }
    }

    return 0;
}