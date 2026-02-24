#include "interrupt.h"
#include <stdint.h>

volatile uint64_t time_elapsed = 0;

#define VGA ((volatile uint16_t*)0xB8000)
static inline void vga_put(int pos, char c) {
  VGA[pos] = (uint16_t)(0x0F << 8) | (uint8_t)c;
}
static inline char hex_digit(uint8_t x){ return (x<10)?('0'+x):('A'+(x-10)); }

typedef struct {
  uint32_t esp;
} task_t;

static task_t tasks[2];
static int cur = 0;

static uint8_t stack1[4096] __attribute__((aligned(16)));
static uint8_t stack2[4096] __attribute__((aligned(16)));

static void taskA(void) {
  uint32_t x = 0;
  while (1) {
    x++;
    // show last hex digit at screen position 0
    vga_put(0, 'A');
    vga_put(1, hex_digit(x & 0xF));
  }
}

static void taskB(void) {
  uint32_t x = 0;
  while (1) {
    x++;
    // show last hex digit at screen position 2
    vga_put(2, 'B');
    vga_put(3, hex_digit(x & 0xF));
  }
}

// Build an initial interrupt-return frame on a new stack
static uint32_t build_initial_stack(uint8_t* stack, void (*entry)(void)) {
  uint32_t* sp = (uint32_t*)(stack + 4096);

  // iret frame: EIP, CS, EFLAGS
  *(--sp) = 0x202;           // EFLAGS (IF=1)
  *(--sp) = 0x08;            // CS
  *(--sp) = (uint32_t)entry; // EIP

  // popa frame (order for popa): edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax
  *(--sp) = 0; // eax
  *(--sp) = 0; // ecx
  *(--sp) = 0; // edx
  *(--sp) = 0; // ebx
  *(--sp) = 0; // esp dummy
  *(--sp) = 0; // ebp
  *(--sp) = 0; // esi
  *(--sp) = 0; // edi

  // segment pops: gs fs es ds
  *(--sp) = 0;
  *(--sp) = 0;
  *(--sp) = 0x10; // es
  *(--sp) = 0x10; // ds

  return (uint32_t)sp;
}

// IRQ0 C handler: acknowledge + switch tasks
uint32_t irq0_c(uint32_t old_esp) {
  // ACK PIC quickly (EOI)
  outb(0x20, 0x20);

  time_elapsed++;

  // save current task esp
  tasks[cur].esp = old_esp;

  // switch every tick
  cur ^= 1;

  return tasks[cur].esp;
}

void kernel_main(void) {
  cli();

  // clear small area
  vga_put(0,'-'); vga_put(1,'-'); vga_put(2,'-'); vga_put(3,'-');

  tasks[0].esp = build_initial_stack(stack1, taskA);
  tasks[1].esp = build_initial_stack(stack2, taskB);

  pic_remap();
  idt_init();
  irq0_install();
  pit_init(100);   // 100Hz timer

  sti();

  // Start by “returning” into taskA via fake esp swap:
  // We just wait; first IRQ0 will switch stacks automatically,
  // but we need one task to begin. So we trigger an int? simplest: just spin.
  while (1) {
    __asm__ volatile("hlt");
  }
}
