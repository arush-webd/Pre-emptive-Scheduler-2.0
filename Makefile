CC=gcc
LD=ld
AS=gcc

CFLAGS=-m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -O2 -Wall -Wextra
ASFLAGS=-m32 -ffreestanding -fno-pie -fno-pic
LDFLAGS=-m elf_i386 -T linker.ld

OBJS=boot.o entry.o kernel.o interrupt.o

all: os.iso

boot.o: boot.s
	$(AS) $(ASFLAGS) -c $< -o $@

entry.o: entry.S
	$(AS) $(ASFLAGS) -c $< -o $@

kernel.o: kernel.c interrupt.h
	$(CC) $(CFLAGS) -c $< -o $@

interrupt.o: interrupt.c interrupt.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

os.iso: kernel.elf
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/kernel.elf
	printf 'set timeout=0\nset default=0\nmenuentry "IRQ0 Demo" {\n  multiboot /boot/kernel.elf\n  boot\n}\n' > iso/boot/grub/grub.cfg
	grub-mkrescue -o os.iso iso >/dev/null 2>&1 || grub2-mkrescue -o os.iso iso
	@echo "Built os.iso"

run: os.iso
	qemu-system-i386 -cdrom os.iso

clean:
	rm -rf *.o kernel.elf iso os.iso
