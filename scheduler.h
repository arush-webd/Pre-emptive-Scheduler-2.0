#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "queue.h"

#define MAX_PROCESSES 32
#define DEFAULT_PRIORITY 1
#define MIN_PRIORITY 0
#define MAX_PRIORITY 10

#define TIMER_HZ 100
#define MS_PER_TICK (1000 / TIMER_HZ)

typedef enum {
	PROCESS_FREE = 0,
	PROCESS_READY,
	PROCESS_RUNNING,
	PROCESS_SLEEPING,
	PROCESS_BLOCKED,
	PROCESS_EXITED
} process_status_t;

/* IMPORTANT: saved_esp must be at offset 0 for entry.s SAVE_STACK/RESTORE_STACK */
typedef struct pcb {
	uint32_t saved_esp;         // offset 0
	int pid;                   // offset 4
	process_status_t status;   // offset 8
	int priority;              // offset 12
	int nested_count;          // offset 16
	uint64_t wakeup_time;      // offset 20 (fine)
	uint32_t kernel_stack_top; // unused here
} pcb_t;

#ifdef __cplusplus
extern "C" {
#endif

void scheduler_init(void);
pcb_t* pcb_allocate(void);
void pcb_free(pcb_t *pcb);

void scheduler_add(pcb_t *pcb);
void scheduler_entry(void);
void put_current_running(void);

void do_sleep(uint32_t milliseconds);
void check_sleeping(void);

pcb_t* get_current_process(void);
queue_t* get_ready_queue(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
