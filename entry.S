.section .text
.global irq0_entry
.extern irq0_c

irq0_entry:
  pusha
  push %ds
  push %es
  push %fs
  push %gs

  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es

  # pass old_esp (= current stack pointer after pushes)
  mov %esp, %eax
  push %eax
  call irq0_c
  add $4, %esp

  # eax = new esp to switch to (or same)
  mov %eax, %esp

  pop %gs
  pop %fs
  pop %es
  pop %ds
  popa
  iret
