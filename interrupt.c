#include "interrupt.h"
#include "common.h"

// From entry.s
extern "C" int disable_count;

// ----------------- Critical sections -----------------
extern "C" void enter_critical(void) {
	cli();
	disable_count++;
}

extern "C" void leave_critical(void) {
	disable_count--;
	if (disable_count == 0) sti();
}

// ----------------- IDT -----------------
typedef struct __attribute__((packed)) {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t  zero;
	uint8_t  type_attr;
	uint16_t offset_high;
} idt_entry_t;

typedef struct __attribute__((packed)) {
	uint16_t limit;
	uint32_t base;
} idt_ptr_t;

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

// assembly entry points from entry.s
extern "C" void irq0_entry(void);
extern "C" void irq7_entry(void);

static void idt_set_gate(int n, uint32_t handler, uint16_t sel, uint8_t flags) {
	idt[n].offset_low  = handler & 0xFFFF;
	idt[n].selector    = sel;
	idt[n].zero        = 0;
	idt[n].type_attr   = flags;
	idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

extern "C" void idt_init(void) {
	for (int i = 0; i < 256; i++) idt_set_gate(i, 0, 0, 0);

	// PIC remapped vectors
	idt_set_gate(32, (uint32_t)irq0_entry, 0x08, 0x8E);
	idt_set_gate(39, (uint32_t)irq7_entry, 0x08, 0x8E);

	idt_ptr.limit = sizeof(idt) - 1;
	idt_ptr.base  = (uint32_t)&idt;

	__asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}

// ----------------- PIC + PIT -----------------
extern "C" void pic_remap(void) {
	uint8_t a1 = inb(0x21);
	uint8_t a2 = inb(0xA1);

	outb(0x11, 0x20);
	outb(0x11, 0xA0);

	outb(0x20, 0x21);
	outb(0x28, 0xA1);

	outb(0x04, 0x21);
	outb(0x02, 0xA1);

	outb(0x01, 0x21);
	outb(0x01, 0xA1);

	outb(a1, 0x21);
	outb(a2, 0xA1);

	// unmask IRQ0
	outb(inb(0x21) & ~0x01, 0x21);
}

extern "C" void pit_init(uint32_t hz) {
	uint32_t divisor = 1193180 / hz;
	outb(0x36, 0x43);
	outb((uint8_t)(divisor & 0xFF), 0x40);
	outb((uint8_t)((divisor >> 8) & 0xFF), 0x40);
}
