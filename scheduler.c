#include "scheduler.h"
#include "interrupt.h"

// From entry.s
extern "C" uint64_t time_elapsed;

queue_t ready_queue;
queue_t sleeping_queue;

// This symbol is used in entry.s macros (SAVE_STACK/RESTORE_STACK/TEST_NESTED_COUNT)
extern "C" pcb_t *current_running = 0;

static pcb_t process_table[MAX_PROCESSES];
static int next_pid = 1;

extern "C" queue_t* get_ready_queue(void) { return &ready_queue; }

extern "C" void scheduler_init(void) {
	queue_init(&ready_queue);
	queue_init(&sleeping_queue);

	for (int i = 0; i < MAX_PROCESSES; i++) {
		process_table[i].saved_esp = 0;
		process_table[i].pid = 0;
		process_table[i].status = PROCESS_FREE;
		process_table[i].priority = DEFAULT_PRIORITY;
		process_table[i].nested_count = 0;
		process_table[i].wakeup_time = 0;
		process_table[i].kernel_stack_top = 0;
	}
	current_running = 0;
}

extern "C" pcb_t* pcb_allocate(void) {
	enter_critical();
	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (process_table[i].status == PROCESS_FREE) {
			process_table[i].pid = next_pid++;
			process_table[i].status = PROCESS_READY;
			process_table[i].priority = DEFAULT_PRIORITY;
			process_table[i].nested_count = 0;
			process_table[i].wakeup_time = 0;
			leave_critical();
			return &process_table[i];
		}
	}
	leave_critical();
	return 0;
}

extern "C" void pcb_free(pcb_t *pcb) {
	if (!pcb) return;
	enter_critical();
	pcb->status = PROCESS_FREE;
	pcb->pid = 0;
	leave_critical();
}

extern "C" void scheduler_add(pcb_t *pcb) {
	if (!pcb) return;
	enter_critical();
	pcb->status = PROCESS_READY;
	queue_put(&ready_queue, pcb);
	leave_critical();
}

extern "C" void scheduler_entry(void) {
	enter_critical();
	pcb_t* next = (pcb_t*)queue_get(&ready_queue);
	if (!next) {
		leave_critical();
		return;
	}
	current_running = next;
	current_running->status = PROCESS_RUNNING;
	// IMPORTANT for irq0 preemption decision
	current_running->nested_count = 0;
	leave_critical();
}

extern "C" void put_current_running(void) {
	enter_critical();
	if (current_running && current_running->status == PROCESS_RUNNING) {
		current_running->status = PROCESS_READY;
		queue_put(&ready_queue, current_running);
	}
	leave_critical();
}

extern "C" void do_sleep(uint32_t milliseconds) {
	enter_critical();
	if (!current_running) {
		leave_critical();
		return;
	}

	uint64_t wakeup_time = time_elapsed + (milliseconds / MS_PER_TICK);
	if (milliseconds % MS_PER_TICK) wakeup_time++;

	current_running->wakeup_time = wakeup_time;
	current_running->status = PROCESS_SLEEPING;
	queue_put(&sleeping_queue, current_running);

	pcb_t* next = (pcb_t*)queue_get(&ready_queue);
	if (next) {
		current_running = next;
		current_running->status = PROCESS_RUNNING;
		current_running->nested_count = 0;
	} else {
		current_running = 0;
	}
	leave_critical();
}

extern "C" void check_sleeping(void) {
	enter_critical();
	int count = queue_size(&sleeping_queue);
	for (int i = 0; i < count; i++) {
		pcb_t* p = (pcb_t*)queue_get(&sleeping_queue);
		if (!p) break;

		if (time_elapsed >= p->wakeup_time) {
			p->status = PROCESS_READY;
			queue_put(&ready_queue, p);
		} else {
			queue_put(&sleeping_queue, p);
		}
	}
	leave_critical();
}

extern "C" pcb_t* get_current_process(void) { return current_running; }
