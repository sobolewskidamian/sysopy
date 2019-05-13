#ifndef ZAD1_DATA_H
#define ZAD1_DATA_H

#include <unistd.h>

#define SHM_NAME "/my_shm"
#define SEM_NAME "/my_sem"
#define SHM_PERM 0600
#define SEM_PERM 0600

#define LOADER_EXECUTABLE "loader.out"

#define MAX_LINE_CAPACITY 1024 // To prevent SHM getting too large
#define ERR_CAP_REACHED 1
#define ERR_WEIGHT_CAP_REACHED 2

struct prod_node
{
    long weight;
    pid_t producer;
    long ordnum;
    __time_t timestamp;
};

struct prod_line
{
    long capacity;
    long weight_capacity;
    long tail;
    long head;
    long size;
    struct prod_node items[MAX_LINE_CAPACITY];
};

struct prod_line *line_new(long, long);
struct prod_node *line_oldest(struct prod_line*);
long line_put(struct prod_line*, long, long);
void line_dispose(struct prod_line*);
long line_weight(struct prod_line*);

#endif //ZAD1_DATA_H
