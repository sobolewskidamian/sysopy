#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "chat.h"

struct Message request;
int q_id;
int clients[MAX_ROOM_SIZE];
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

    int client_qid = request.num1;
    clients[i] = client_qid;
    request.num1 = i;
    removeActiveFriends(i);

    msgsnd(client_qid, &request, sizeof(struct Message) - sizeof(long), 0);
}

void respondStop() {
    int client_qid = request.num2;
    request.type = request.req_type = STOP;
    for (int i = 0; i < MAX_ROOM_SIZE; i++)
        if (clients[i] == client_qid) clients[i] = 0;

    request.num1 = -1;
    request.num2 = -1;

    msgsnd(client_qid, &request, sizeof(struct Message) - sizeof(long), 0);
}

void respondEcho() {
    int client_qid = clients[request.num1];
    request.type = request.req_type = ECHO;
    timestampMessage(request.num1, request.arg1);

    msgsnd(client_qid, &request, sizeof(struct Message) - sizeof(long), 0);
}

void respondToAll() {
    request.type = request.req_type = ECHO;
    timestampMessage(request.num1, request.arg1);

    for (int i = 0; i < MAX_ROOM_SIZE; i++) {
        int qid = clients[i];
        if (qid == 0)
            continue;

        msgsnd(qid, &request, sizeof(struct Message) - sizeof(long), 0);
    }
}

void respondToFriends() {
    request.type = request.req_type = ECHO;
    timestampMessage(request.num1, request.arg1);

    for (int i = 0; i < MAX_ROOM_SIZE && friends[request.num1][i] != -1; i++) {
        int qid = clients[friends[request.num1][i]];
        if (qid == 0)
            continue;

        msgsnd(qid, &request, sizeof(struct Message) - sizeof(long), 0);
    }
}

void respondToOne() {
    request.type = request.req_type = ECHO;
    timestampMessage(request.num1, request.arg1);

    int qid = clients[request.num2];
    if (qid == 0)
        return;

    msgsnd(qid, &request, sizeof(struct Message) - sizeof(long), 0);
}

void respondList() {
    int client_qid = clients[request.num1];
    request.type = request.req_type = LIST;

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

    msgsnd(client_qid, &request, sizeof(struct Message) - sizeof(long), 0);
}

void respondFriends() {
    int client_qid = clients[request.num1];

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

            for (int i = 0; i < MAX_ROOM_SIZE; i++) {
                if (friends[request.num1][i] == -1) {
                    friends[request.num1][i] = atoi(token);
                    token = strtok(NULL, " ");
                    break;
                }
            }
        }
    } else if (strcmp(token, "DEL") == 0) {
        token = strtok(NULL, " ");

        while (token != NULL) {
            for (int i = 0; i < MAX_ROOM_SIZE; i++) {
                if (friends[request.num1][i] == atoi(token)) {
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
            friends[request.num1][i++] = atoi(token);
            token = strtok(NULL, " ");
        }
    }

    msgsnd(client_qid, &request, sizeof(struct Message) - sizeof(long), 0);
}

void handleExit() {
    request.type = request.req_type = STOP;

    for (int i = 0; i < MAX_ROOM_SIZE; i++) {
        if (clients[i] == 0)
            continue;
        msgsnd(clients[i], &request, sizeof(struct Message) - sizeof(long), 0);
    }

    msgctl(q_id, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char **argv) {
    memset(clients, 0, sizeof(clients));
    for (int i = 0; i < MAX_ROOM_SIZE; i++)
        for (int j = 0; j < MAX_ROOM_SIZE; j++)
            friends[i][j] = -1;

    key_t key = ftok(SERVER_KEY_PATH, SERVER_KEY_SEED);
    q_id = msgget(key, IPC_CREAT | QUEUE_PERMISSIONS);

    signal(SIGINT, handleExit);

    printf("========Chat server=========\n");
    printf("Queue ID: %d\nListening...\n", q_id);

    while (1) {
        msgrcv(q_id, &request, sizeof(struct Message) - sizeof(long), BY_PRIORITY, 0);

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