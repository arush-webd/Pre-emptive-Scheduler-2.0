#pragma once
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void sti(void){ __asm__ volatile("sti"); }
static inline void cli(void){ __asm__ volatile("cli"); }

void idt_init(void);
void pic_remap(void);
void pit_init(uint32_t hz);

void irq0_install(void);

/* called from assembly */
uint32_t irq0_c(uint32_t old_esp);

/* debug tick counter */
extern volatile uint64_t time_elapsed;
