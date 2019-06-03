#ifndef ZAD1_LIB_H
#define ZAD1_LIB_H

#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <pthread.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr_un sockaddr_un;
typedef struct epoll_event epoll_event;

typedef enum {
    /* When client registers in server */
            MSG_REGISTER,
    /* When server asks client for doing job */
            MSG_REQUEST,
    /* When client ends computation and sends response to server.*/
            MSG_RESPONSE,
    /* When client decides to quit server */
            MSG_QUIT
} msg_type;

typedef struct message {
    msg_type type;
    /* Fields used by some certain types to pass extra data. */
    char buff[4096];
    long num;
    long num_sec;
} message;

#endif //ZAD1_LIB_H
