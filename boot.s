.section .multiboot
.align 4
.long 0x1BADB002
.long 0x00000000
.long -(0x1BADB002 + 0x00000000)

.section .bss
.align 16
stack_bottom:
  .skip 16384
stack_top:

.section .text
.global _start
.extern kernel_main

_start:
  mov $stack_top, %esp

  # GRUB puts: eax=magic, ebx=multiboot_info. We ignore for demo.
  call kernel_main

.hang:
  cli
  hlt
  jmp .hang
