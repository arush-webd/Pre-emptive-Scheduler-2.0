#include "interrupt.h"

#define IDT_ENTRIES 256

struct __attribute__((packed)) idt_entry {
  uint16_t base_lo;
  uint16_t sel;
  uint8_t  always0;
  uint8_t  flags;
  uint16_t base_hi;
};

struct __attribute__((packed)) idt_ptr {
  uint16_t limit;
  uint32_t base;
};

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr   idtp;

extern void irq0_entry(void);  // from entry.S

static void idt_set_gate(int n, uint32_t base, uint16_t sel, uint8_t flags) {
  idt[n].base_lo = base & 0xFFFF;
  idt[n].base_hi = (base >> 16) & 0xFFFF;
  idt[n].sel     = sel;
  idt[n].always0 = 0;
  idt[n].flags   = flags;
}

static inline void idt_load(struct idt_ptr* p) {
  __asm__ volatile("lidt (%0)" : : "r"(p));
}

void idt_init(void) {
  idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
  idtp.base  = (uint32_t)&idt;

  for (int i = 0; i < IDT_ENTRIES; i++) {
    idt_set_gate(i, 0, 0, 0);
  }

  // IRQ0 is 0x20 after PIC remap
  idt_set_gate(0x20, (uint32_t)irq0_entry, 0x08, 0x8E);

  idt_load(&idtp);
}

// Remap PIC to 0x20..0x2F
void pic_remap(void) {
  uint8_t a1 = inb(0x21);
  uint8_t a2 = inb(0xA1);

  outb(0x20, 0x11);
  outb(0xA0, 0x11);

  outb(0x21, 0x20);
  outb(0xA1, 0x28);

  outb(0x21, 0x04);
  outb(0xA1, 0x02);

  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  outb(0x21, a1);
  outb(0xA1, a2);
}

void pit_init(uint32_t hz) {
  // PIT base = 1193182 Hz
  uint32_t div = 1193182 / hz;
  outb(0x43, 0x36);
  outb(0x40, (uint8_t)(div & 0xFF));
  outb(0x40, (uint8_t)((div >> 8) & 0xFF));
}

void irq0_install(void) {
  // Unmask IRQ0 on PIC master
  uint8_t mask = inb(0x21);
  mask &= ~(1 << 0);
  outb(0x21, mask);
}
