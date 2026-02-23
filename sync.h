#ifndef SYNC_H
#define SYNC_H

#include "queue.h"
#include "scheduler.h"

typedef struct {
	int value;
	queue_t waiters;
} sem_t;

typedef struct {
	queue_t waiters;
} cond_t;

typedef struct {
	int total;
	int count;
	queue_t waiters;
} barrier_t;

#ifdef __cplusplus
extern "C" {
#endif

void sem_init(sem_t *s, int initial);
void sem_wait(sem_t *s);
void sem_signal(sem_t *s);

void cond_init(cond_t *c);
void cond_wait(cond_t *c);
void cond_signal(cond_t *c);
void cond_broadcast(cond_t *c);

void barrier_init(barrier_t *b, int total_threads);
void barrier_wait(barrier_t *b);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
