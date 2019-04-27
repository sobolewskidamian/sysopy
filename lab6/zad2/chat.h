#ifndef ZAD2_CHAT_H
#define ZAD2_CHAT_H


#define MAX_MESSAGE_SIZE 512
#define SERVER_NAME "/aa"
#define QUEUE_PERMISSIONS 0666
#define MAX_ROOM_SIZE 32


enum Type {
    STOP = 1,
    LIST = 2,
    FRIENDS = 3,
    INIT = 4,
    ECHO = 5,
    ECHO_TO_ALL = 6,
    ECHO_TO_FRIENDS = 7,
    ECHO_TO_PERSON = 8,
};


struct Message {
    enum Type req_type;
    int num1;
    int num2;
    char arg1[MAX_MESSAGE_SIZE];
    char arg2[MAX_MESSAGE_SIZE];
};


#endif //ZAD2_CHAT_H
