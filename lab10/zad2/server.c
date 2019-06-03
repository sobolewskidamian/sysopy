#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include "lib.h"

#define UNIX_PATH_MAX    108
#define MAX_CLIENTS      16
#define MAX_EPOLL_EVENTS 128

static unsigned short af_inet_port;
static char af_unix_path[UNIX_PATH_MAX];

static volatile int RUNNING = 1;

static int client_fds[MAX_CLIENTS] = {0};  // Holds FDs of connected sockets.
static int client_busy[MAX_CLIENTS] = {0}; // Tells if FD is busy.
static void *client_addr[MAX_CLIENTS];
static char client_hostname[MAX_CLIENTS][1 << 6]; // Hostname of FD.
static int requests_sent = 0;

void exit_handler(int sig) {
    RUNNING = 0;
}

pthread_t *spawn_thread(void *(*thread_main)(void *)) {
    pthread_t *tid = malloc(sizeof(pthread_t));
    pthread_create(tid, NULL, thread_main, NULL);
    return tid;
}

void join_thread(pthread_t *tid) {
    void *ret;
    pthread_join(*tid, &ret);
    free(tid);
}

int add_epoll(int epoll_fd, int fd) {
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    return 0;
}

int register_client(int fd, const char *hostname) {
    int i = 0;
    while (i < MAX_CLIENTS) {
        if (client_fds[i] == 0) {
            client_fds[i] = fd;
            client_busy[i] = 0;
            strcpy(client_hostname[i], hostname);
            return i;
        }
        i++;
    }
    return -1;
}

void handle_event(epoll_event *event) {
    message msg;
    size_t bytes_read;
    int i;

    int fd = event->data.fd;
    sockaddr *addr = malloc(sizeof(sockaddr));
    socklen_t len = sizeof(sockaddr);
    bytes_read = recvfrom(fd, &msg, sizeof(msg), 0, addr, &len);

    if (bytes_read == sizeof(message)) {
        switch (msg.type) {
            case MSG_REGISTER:
                i = register_client(fd, msg.buff);
                client_addr[i] = malloc(sizeof(sockaddr));
                memcpy(client_addr[i], addr, sizeof(sockaddr));

                printf("======= WELCOME %s =======\n", msg.buff);
                printf("------------------------\n");
                break;
            case MSG_QUIT:
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (strcmp(client_hostname[i], msg.buff) == 0) {
                        printf("======= RIP %s =======\n", client_hostname[i]);
                        printf("------------------------\n");
                        client_fds[i] = client_busy[i] = 0;
                        free(client_addr[i]);
                        client_addr[i] = NULL;
                        memset(client_hostname[i], 0, sizeof(client_hostname[i]));
                        break;
                    }
                }
                break;
            case MSG_RESPONSE:
                printf("======== %ld RESPONSE ========\n", msg.num);
                printf("WORD COUNT: %ld\n", msg.num_sec);
                printf("------------------------\n");

                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (strcmp(client_hostname[i], msg.buff) == 0) {
                        client_busy[i]--;
                        break;
                    }
                }
                break;
            default:
                break;
        }
    }

    free(addr);
}


void *listener_thread(void *arg) {
    sockaddr_in inet_addr;
    sockaddr_un unix_addr;

    int epoll_fd, event_count;
    epoll_event events[MAX_EPOLL_EVENTS];

    // AF_INET UDP socket creation
    int inet_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (inet_fd == -1) {
        printf("AF_INET: Socket creation failed\n");
        exit(1);
    } else
        printf("AF_INET: Socket successfully created!\n");

    // AF_INET - make socket non-blocking
    int flags = fcntl(inet_fd, F_GETFL);
    fcntl(inet_fd, F_SETFL, flags | O_NONBLOCK);

    memset(&inet_addr, 0, sizeof(inet_addr));
    inet_addr.sin_family = AF_INET;
    inet_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_addr.sin_port = htons(af_inet_port);

    // AF_INET binding
    if ((bind(inet_fd, (sockaddr *) &inet_addr, sizeof(inet_addr))) != 0) {
        printf("AF_INET: Socket bind failed\n");
        exit(1);
    } else
        printf("AF_INET: Socket successfully binded!\n");

    //AF_UNIX socket creation
    int unix_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (unix_fd == -1) {
        printf("AF_UNIX: Socket creation failed\n");
        exit(1);
    } else
        printf("AF_UNIX: Socket successfully created!\n");

    // AF_UNIX - make socket non-blocking
    flags = fcntl(unix_fd, F_GETFL);
    fcntl(unix_fd, F_SETFL, flags | O_NONBLOCK);

    memset(&unix_addr, 0, sizeof(unix_addr));
    unix_addr.sun_family = AF_UNIX;
    strcpy(unix_addr.sun_path, af_unix_path);

    // AF_UNIX binding
    if ((bind(unix_fd, (sockaddr *) &unix_addr, sizeof(unix_addr))) != 0) {
        printf("AF_UNIX: Socket bind failed\n");
        exit(1);
    } else
        printf("AF_UNIX: Socket successfully binded!\n");

    // Create EPOLL FD
    if ((epoll_fd = epoll_create1(0)) == -1) {
        printf("Failed to create epoll file descriptor\n");
        exit(1);
    }

    add_epoll(epoll_fd, inet_fd);
    add_epoll(epoll_fd, unix_fd);

    while (RUNNING) {
        // Monitor epool events.
        event_count = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, 0);
        for (int i = 0; i < event_count; i++)
            handle_event(&events[i]);
    }

    // Close socket before finishing
    close(inet_fd);
    close(unix_fd);
    close(epoll_fd);
    unlink(af_unix_path);

    printf("Listener stopping...\n");
    exit(0);
}

void *input_thread(void *arg) {
    message msg;
    char path[1 << 8];
    int fp;

    memset(&msg, 0, sizeof(message));
    msg.type = MSG_REQUEST;

    while (RUNNING) {
        scanf("%s", path);

        fp = open(path, O_RDONLY);
        if (fp < 0) {
            printf("Unable to open %s\n", path);
            continue;
        }

        if (read(fp, msg.buff, sizeof(msg.buff)) < 0) {
            printf("Error while reading %s\n", path);
            continue;
        }

        msg.num = requests_sent++;

        int cli_idx, cli_fd;
        cli_idx = -1;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] == 0)
                continue;

            cli_idx = i;
            if (!client_busy[i])
                break;
        }

        cli_fd = client_fds[cli_idx];
        if (cli_idx == -1 || cli_fd == 0) {
            printf("No availablie clients\n");
            continue;
        }

        if (sendto(cli_fd, &msg, sizeof(msg), 0, (sockaddr *) client_addr[cli_idx], sizeof(sockaddr)) < 0)
            printf("Error sending request to FD %s\n", client_hostname[cli_idx]);
        else {
            client_busy[cli_idx]++;
            printf("======= REQUEST %ld =========\n", msg.num);
            printf("WORKER: %d\n", cli_fd);
            printf("-----------------------------\n");
        }
        memset(&msg.buff, 0, sizeof(msg.buff));
    }

    printf("Input thread stopping...\n");
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Bad amount of arguments.\n");
        exit(1);
    }

    af_inet_port = (unsigned short) strtoul(argv[1], NULL, 10);
    strcpy(af_unix_path, argv[2]);
    signal(SIGINT, exit_handler);

    pthread_t *listener_tid = spawn_thread(listener_thread);
    pthread_t *input_tid = spawn_thread(input_thread);

    join_thread(listener_tid);
    join_thread(input_tid);

    return 0;
}