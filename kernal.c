#include <stdint.h>
#include "interrupt.h"
#include "scheduler.h"

// From entry.s
extern "C" uint64_t time_elapsed;
extern "C" int disable_count;

// VGA text buffer
static volatile uint16_t* const VGA = (uint16_t*)0xB8000;
static int row = 0;

static void vga_puts(const char* s) {
	int col = 0;
	while (*s) {
		VGA[row * 80 + col] = (uint16_t)(0x0F << 8) | (uint8_t)(*s);
		col++;
		s++;
	}
	row = (row + 1) % 25;
}

static void vga_putu(uint32_t x) {
	char buf[16];
	int i = 0;
	if (x == 0) { vga_puts("0"); return; }
	while (x > 0) {
		buf[i++] = char('0' + (x % 10));
		x /= 10;
	}
	for (int j = i - 1; j >= 0; j--) {
		char tmp[2] = { buf[j], 0 };
		vga_puts(tmp);
	}
}

// IMPORTANT: called from boot.s => keep C ABI name "kernel_main"
extern "C" void kernel_main(void) {
	cli();
	disable_count = 0;

	vga_puts("Kernel booting...");

	scheduler_init();

	// Dummy current_running so entry.s TEST_NESTED_COUNT doesn't dereference NULL
	pcb_t* p = pcb_allocate();
	if (p) {
		p->status = PROCESS_RUNNING;
		p->nested_count = 1; // nonzero => irq0 takes nested_not_zero path in your current entry.s
		extern "C" pcb_t* current_running;
		current_running = p;
	}

	pic_remap();
	idt_init();
	pit_init(100); // 100 Hz => 10ms tick

	vga_puts("IDT/PIC/PIT initialized.");
	vga_puts("Enabling interrupts...");

	sti();

	uint64_t last = 0;
	while (1) {
		// Print once per second (100 ticks)
		if (time_elapsed - last >= 100) {
			last = time_elapsed;
			vga_puts("Tick (time_elapsed low32):");
			vga_putu((uint32_t)time_elapsed);
		}
		__asm__ volatile ("hlt");
	}
}
