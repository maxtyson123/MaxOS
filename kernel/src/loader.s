.section .data
boot_stack_base:
    .byte 0x1000

.section .mb_text
.extern kernelMain
.extern callConstructors

_start :
    ; // Backup the multiboot header pointer and magic number
    mov %ebx, %edi
    mov %eax, %esi

    ; // setup a stack, and reset flags
    mov $(boot_stack_base + 0x1000), %esp
    push 0x2
    popf

    ; // TODO: Paging

    ; // load cr3
    mov pml4_addr, %eax
    mov %eax, %cr3

    ; // enable PAE
    mov $0x20, %eax
    mov %eax, %cr4

    ; // set LME
    mov $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr

    ; // now we're ready to enable paging, and jump to long mode
    mov %cr0, %eax
    orl $(1 << 31)
    mov %eax, %cr0

    ; // Load the GDT
    lgdt [gdt64.ptr]

    ; // Update the segment registers
    mov gdt64.data, %ax
    mov %ax, %ss                ; // Stack segment
    mov %ax, %ds                ; // Data segment
    mov %ax, %es                ; // Extra segment
    mov %ax, %fs                ; // Extra segment
    mov %ax, %gs                ; // Extra segment

    ; // Load the code segment
    jmp *gdt64_code_long_mode_entry

gdt64_code_long_mode_entry:
    .quad gdt64.code:long_mode_entry

long_mode_entry:
    call callConstructors
    call kernel_main
    hlt

gdt64:
    .quad 0                   ; // Null segment
.code:
    ; // Descriptor (bit 44) = 1, Present (bit 47) = 1, Read/Writable (bit 41) = 1, Execute (bit 43) = 1, 64-bit (bit 53) = 1
    .quad (1 << 44) | (1 << 47) | (1 << 41) | (1 << 43) | (1 << 53)

.data:
    ; // Descriptor (bit 44) = 1, Present (bit 47) = 1, Read/Writable (bit 41) = 1
    .quad (1 << 44) | (1 << 47) | (1 << 41)

.ptr:
    .word .ptr - gdt64 - 1
    .long gdt64
