#include "sync.h"
#include "interrupt.h"

extern pcb_t *current_running;

static void make_ready(pcb_t *p) {
    if (!p) return;
    p->status = PROCESS_READY;
    queue_put(get_ready_queue(), p);
}

static void block_current_on(queue_t *q) {
    pcb_t *curr = current_running;
    if (!curr) return;

    curr->status = PROCESS_BLOCKED;
    queue_put(q, curr);

    pcb_t *next = (pcb_t*)queue_get(get_ready_queue());
    if (next) {
        current_running = next;
        current_running->status = PROCESS_RUNNING;
        current_running->nested_count = 0;
    } else {
        current_running = 0;
    }
}

static void wake_one(queue_t *q) {
    pcb_t *p = (pcb_t*)queue_get(q);
    if (p) make_ready(p);
}

static void wake_all(queue_t *q) {
    int n = queue_size(q);
    for (int i = 0; i < n; i++) {
        pcb_t *p = (pcb_t*)queue_get(q);
        if (!p) break;
        make_ready(p);
    }
}

void sem_init(sem_t *s, int initial) {
    enter_critical();
    s->value = initial;
    queue_init(&s->waiters);
    leave_critical();
}

void sem_wait(sem_t *s) {
    enter_critical();
    if (s->value > 0) {
        s->value--;
        leave_critical();
        return;
    }
    block_current_on(&s->waiters);
    leave_critical();
}

void sem_signal(sem_t *s) {
    enter_critical();
    if (queue_size(&s->waiters) > 0) wake_one(&s->waiters);
    else s->value++;
    leave_critical();
}

void cond_init(cond_t *c) {
    enter_critical();
    queue_init(&c->waiters);
    leave_critical();
}

void cond_wait(cond_t *c) {
    enter_critical();
    block_current_on(&c->waiters);
    leave_critical();
}

void cond_signal(cond_t *c) {
    enter_critical();
    wake_one(&c->waiters);
    leave_critical();
}

void cond_broadcast(cond_t *c) {
    enter_critical();
    wake_all(&c->waiters);
    leave_critical();
}

void barrier_init(barrier_t *b, int total_threads) {
    enter_critical();
    b->total = (total_threads < 1) ? 1 : total_threads;
    b->count = 0;
    queue_init(&b->waiters);
    leave_critical();
}

void barrier_wait(barrier_t *b) {
    enter_critical();
    b->count++;
    if (b->count < b->total) {
        block_current_on(&b->waiters);
        leave_critical();
        return;
    }
    b->count = 0;
    wake_all(&b->waiters);
    leave_critical();
}
