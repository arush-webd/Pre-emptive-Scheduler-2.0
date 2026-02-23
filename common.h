#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

/*
 * ----- PCB layout must match entry.s expectations -----
 * entry.s uses:
 *  - SAVE_STACK: movl %esp, (%eax) => pcb->saved_esp at offset 0
 *  - TEST_NESTED_COUNT: NESTED_COUNT_OFFSET(%eax)
 */
#define NESTED_COUNT_OFFSET 16

static inline void outb(uint8_t val, uint16_t port) {
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

#endif
