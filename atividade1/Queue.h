#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
    char data[256];
    int head;
    int tail;
} Queue;

void queue_init(Queue *q);
bool queue_put(Queue *q, char c);
bool queue_get(Queue *q, char *c);
int queue_size(Queue *q);

#endif