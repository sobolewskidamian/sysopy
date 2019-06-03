#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lib.h"

#define NIX_SOCK 1
#define NET_SOCK 2

int connect_inet(int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        printf("INET: socket creation failed\n");
        exit(1);
    } else
        printf("INET: socket successfully created\n");

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(port);

    if (connect(sock_fd, (sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        printf("INET: Connection with the server failed\n");
        exit(1);
    } else
        printf("INET: Connected to the server\n");

    return sock_fd;
}

int connect_unix(const char *sockpath) {
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        printf("UNIX: Socket creation failed\n");
        exit(2);
    } else
        printf("UNIX: Socket successfully created!\n");

    sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, sockpath);

    if ((connect(sock_fd, (sockaddr *) &server_addr, sizeof(server_addr))) != 0) {
        printf("UNIX: Connection failed\n");
        exit(2);
    } else
        printf("UNIX: Aquired connection!\n");

    return sock_fd;
}

int count_words(char *text) {
    int count = 0;
    char *token = strtok(text, " ");
    while (token != NULL) {
        count++;
        token = strtok(NULL, " ");
    }

    return count;
}

int client_loop(int fd, const char *hostname) {
    //Send init message
    message msg;
    msg.type = MSG_REGISTER;
    strcpy(msg.buff, hostname);
    send(fd, &msg, sizeof(msg), 0);

    while (1) {
        if (recv(fd, &msg, sizeof(msg), MSG_DONTWAIT) < 0)
            continue;

        switch (msg.type) {
            case MSG_PING:
                printf("PINGED\n");
                break;
            case MSG_REQUEST:
                printf("======= REQUEST %ld =======\n", msg.num);
                printf("---------------------------\n");

                int word_count = count_words(msg.buff);
                msg.type = MSG_RESPONSE;
                msg.num_sec = word_count;

                if (send(fd, &msg, sizeof(msg), MSG_NOSIGNAL) < 0)
                    printf("Error sending request to FD %d\n", fd);
                else {
                    printf("======= RESPONSE %ld =======\n", msg.num);
                    printf("FD: %d\n", fd);
                    printf("---------------------------\n");
                }
                break;
            default:
                break;
        }

        memset(&msg, 0, sizeof(message));
    }
}

int main(int argc, char **argv) {
    int sock_type;
    int sock_fd;

    if (argc != 4) {
        printf("Bad amount of arguments.\n");
        exit(1);
    }

    char *hostname = argv[1];
    char *address = argv[3];
    if (strcmp(argv[2], "NET") == 0)
        sock_type = NET_SOCK;
    else
        sock_type = NIX_SOCK;

    if (sock_type & NET_SOCK) {
        int port = (int) strtol(address, NULL, 10);
        if (port < 0 || port >= 1 << 16) {
            printf("INET port must be in range %d-%d.", 0, 1 << 16);
            exit(1);
        }
        sock_fd = connect_inet(port);
    } else
        sock_fd = connect_unix(address);

    return client_loop(sock_fd, hostname);
}