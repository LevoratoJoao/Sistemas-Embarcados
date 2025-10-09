#include "Queue.h"

void queue_init(Queue *q) {
    q->head = 0;
    q->tail = 0;
}

bool queue_put(Queue *q, char c) {
    int next = (q->head + 1) % 256;
    if (next == q->tail)
        return false;
    q->data[q->head] = c;
    q->head = next;
    return true;
}

bool queue_get(Queue *q, char *c) {
    if (q->head == q->tail)
        return false;
    *c = q->data[q->tail];
    q->tail = (q->tail + 1) % 256;
    return true;
}

int queue_size(Queue *q) {
    return (q->head - q->tail + 256) % 256;
}