.set MAGIC,          0x1BADB002  ; // The magic number
.set ALIGN,          1 << 0       ; // Align modules on page boundaries
.set MEMINFO,        1 << 1       ; // Provide memory map
.set VESA_ENABLED,   1 << 2       ; // Enable VESA support
.set VESA_DISABLED,  0 << 2       ; // Disable VESA support
.set FLAGS,          ALIGN | MEMINFO | VESA_DISABLED
.set CHECKSUM,       -(MAGIC + FLAGS)

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long 0, 0, 0, 0, 0
    .long 0                     ; //  0 = set graphics mode
    .long 1024, 768, 32         ; // Width, height, depth

.section .text
.extern kernelMain
.extern callConstructors
.global loader

loader:
    mov $kernel_stack, %esp     ; // Init the stack
    push %eax                   ; // Save the magic number on the stack
    push %ebx                   ; // Save the address of the multiboot structure on the stack
    call callConstructors       ; // Init the constructors
    call kernelMain

_stop:
    cli
    hlt
    jmp _stop

.section .bss
.space 4*1024*1024              ; // The kernel stack (4MB)
kernel_stack:
