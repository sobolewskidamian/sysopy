#include <stdlib.h>
#include <sys/time.h>
#include "data.h"

struct prod_line *line_new(long capacity, long weight) {
    struct prod_line *line = (struct prod_line *) malloc(sizeof(struct prod_line));
    if (line == NULL) {
        return NULL;
    }

    line->capacity = capacity;
    line->weight_capacity = weight;
    line->tail = 0;
    line->head = 0;
    line->size = 0;

    return line;
}

struct prod_node *line_oldest(struct prod_line *line) {
    if (line->size == 0) {
        return NULL;
    }

    struct prod_node *item = line->items + line->head;
    line->size--;
    line->head++;

    if (line->head >= MAX_LINE_CAPACITY)
        line->head = 0;

    return item;
}

long line_put(struct prod_line *line, long weight, long ordnum) {
    if (line->capacity < line->size + 1 || MAX_LINE_CAPACITY < line->size + 1)
        return ERR_CAP_REACHED;

    if (line->weight_capacity < line_weight(line) + weight)
        return ERR_WEIGHT_CAP_REACHED;


    struct prod_node *item = line->items + line->tail;
    item->weight = weight;
    item->ordnum = ordnum;
    item->producer = getpid();

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    item->timestamp = current_time.tv_sec * (int) 1e6 + current_time.tv_usec;

    line->size++;
    line->tail++;
    if (line->tail >= MAX_LINE_CAPACITY)
        line->tail = 0;

    return 0;
}

void line_dispose(struct prod_line *line) {
    free(line);
}

long line_weight(struct prod_line *line) {
    long i = line->head;
    long weight = 0;

    for (int j = 0; j < line->size; j++) {
        weight += line->items[i].weight;
        i++;
        if (i >= MAX_LINE_CAPACITY)
            i = 0;
    }

    return weight;
}

