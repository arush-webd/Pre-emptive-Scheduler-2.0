#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void enter_critical(void);
void leave_critical(void);

void idt_init(void);
void pic_remap(void);
void pit_init(uint32_t hz);

#ifdef __cplusplus
} // extern "C"
#endif

static inline void sti(void) { __asm__ volatile ("sti"); }
static inline void cli(void) { __asm__ volatile ("cli"); }

#endif
