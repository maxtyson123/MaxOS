.set MAGIC, 0x1badb002
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM


.section .text
.extern kernelMain
.extern callConstructors
.global loader

loader:
    mov $kernel_stack, %esp     ; // Init the stack
    push %eax                   ; // Save the magic number on the stack
    push %ebx                   ; // Save the address of the multiboot structure on the stack
    call callConstructors       ; // Init theconstructors
    call kernelMain

_stop:
    cli
    hlt
    jmp _stop

.section .bss
.space 4*1024*1024              ; // The kernel stack (4MB)
kernel_stack:
