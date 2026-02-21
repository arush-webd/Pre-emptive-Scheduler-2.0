CC = gcc
AS = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -O2 -Wall -Wextra
ASFLAGS = -m32 -ffreestanding -fno-pie -fno-pic
LDFLAGS = -m elf_i386 -T linker.ld

OBJS = boot.o kernel.o interrupt.o scheduler.o sync.o entry.o

all: os.iso

boot.o: boot.s
	$(AS) $(ASFLAGS) -c $< -o $@

entry.o: entry.s
	$(AS) $(ASFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

os.iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	printf 'set timeout=0\nset default=0\nmenuentry "os" {\n  multiboot /boot/kernel.bin\n  boot\n}\n' > iso/boot/grub/grub.cfg
	grub-mkrescue -o os.iso iso >/dev/null 2>&1 || grub2-mkrescue -o os.iso iso
	@echo "Built os.iso"

run: os.iso
	qemu-system-i386 -cdrom os.iso

clean:
	rm -rf *.o kernel.bin iso os.iso
