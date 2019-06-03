#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "lib.h"

#define NIX_SOCK 1
#define NET_SOCK 2
#define CLI_NAME "/tmp/cli"

static volatile int RUNNING = 1;

int connect_inet(void) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock_fd == -1) {
        printf("INET: socket creation failed\n");
        exit(1);
    } else
        printf("INET: socket successfully created\n");

    return sock_fd;
}

sockaddr *get_destination_addr(int type, void *identifier) {
    sockaddr *addr = malloc(sizeof(sockaddr));
    memset(addr, 0, sizeof(sockaddr));

    if (type & NIX_SOCK) {
        sockaddr_un unix_addr;
        char *sockpath = (char *) identifier;
        memset(&unix_addr, 0, sizeof(unix_addr));
        unix_addr.sun_family = AF_UNIX;
        strcpy(unix_addr.sun_path, sockpath);
        memcpy(addr, &unix_addr, sizeof(sockaddr));
    } else {
        sockaddr_in inet_addr;
        int port = *((int *) identifier);
        memset(&inet_addr, 0, sizeof(inet_addr));
        inet_addr.sin_family = AF_INET;
        inet_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        inet_addr.sin_port = htons(port);
        memcpy(addr, &inet_addr, sizeof(sockaddr));
    }
    return addr;
}

int connect_unix(void) {
    int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        printf("UNIX: Socket creation failed\n");
        exit(2);
    } else
        printf("UNIX: Socket successfully created!\n");

    sockaddr_un client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, CLI_NAME);

    if (bind(sock_fd, (sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
        printf("UNIX: Unable to bind");
        exit(1);
    }

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

int client_loop(int fd, int type, void *addr, const char *hostname) {
    sockaddr cli;
    sockaddr *dest = get_destination_addr(type, addr);
    socklen_t server_addr_len = sizeof(sockaddr);
    //Send init message
    message msg;
    msg.type = MSG_REGISTER;
    strcpy(msg.buff, hostname);

    if (sendto(fd, &msg, sizeof(message), 0, dest, sizeof(sockaddr)) != sizeof(msg)) {
        printf("Error while registering at server\n");
        exit(1);
    }

    while (RUNNING) {
        if (recvfrom(fd, &msg, sizeof(message), MSG_DONTWAIT, &cli, &server_addr_len) < 0)
            continue;

        switch (msg.type) {
            case MSG_REQUEST:
                printf("======= REQUEST %ld =======\n", msg.num);
                printf("---------------------------\n");

                int word_count = count_words(msg.buff);
                msg.type = MSG_RESPONSE;
                msg.num_sec = word_count;

                //sleep(1);
                if (sendto(fd, &msg, sizeof(msg), 0, dest, sizeof(sockaddr)) < 0)
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

    msg.type = MSG_QUIT;
    strcpy(msg.buff, hostname);
    sendto(fd, &msg, sizeof(message), 0, dest, sizeof(sockaddr));

    close(fd);
    return 0;
}

void exit_handler(int sig) {
    RUNNING = 0;
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

    signal(SIGINT, exit_handler);

    if (sock_type & NET_SOCK) {
        int port = (int) strtol(address, NULL, 10);
        if (port < 0 || port >= 1 << 16) {
            printf("INET port must be in range %d-%d.", 0, 1 << 16);
            exit(1);
        }
        sock_fd = connect_inet();
        return client_loop(sock_fd, NET_SOCK, &port, hostname);
    } else {
        sock_fd = connect_unix();
        return client_loop(sock_fd, NIX_SOCK, address, hostname);
    }
}