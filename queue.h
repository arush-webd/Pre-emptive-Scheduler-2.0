#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

#define QUEUE_CAPACITY 128

typedef struct {
    void* items[QUEUE_CAPACITY];
    int head, tail, size;
} queue_t;

static inline void queue_init(queue_t* q) {
    q->head = q->tail = q->size = 0;
}

static inline int queue_size(queue_t* q) {
    return q->size;
}

static inline int queue_put(queue_t* q, void* item) {
    if (q->size >= QUEUE_CAPACITY) return -1;
    q->items[q->tail] = item;
    q->tail = (q->tail + 1) % QUEUE_CAPACITY;
    q->size++;
    return 0;
}

static inline void* queue_get(queue_t* q) {
    if (q->size <= 0) return 0;
    void* item = q->items[q->head];
    q->head = (q->head + 1) % QUEUE_CAPACITY;
    q->size--;
    return item;
}

#endif
