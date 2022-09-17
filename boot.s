/* Constants for multiboot header */
.set ALIGN, 1<<0     /*align loaded modules for page boundries*/
.set MEMINFO, 1<<0   /*Memory Map*/
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/* Multi Boot header, this marks the program as a kernel */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* The stack*/
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/* The linker makes start (below ) the start point of the kernel and the boot loader will jump to this position once loaded */
.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp
    call kernel_main
    /*Nothing More to do so loop*/
    cli
1:	hlt
	jmp 1b

.size _start, . - _start

/* Future Refrence https://wiki.osdev.org/Bare_Bones*/

