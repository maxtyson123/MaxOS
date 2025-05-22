%define KERNEL_VIRTUAL_ADDR 0xFFFFFFFF80000000
%define PAGE_SIZE 0x1000
%define FLAGS 0b10 | 1
%define LOOP_LIMIT 1024
%define PD_LOOP_LIMIT 2


global p2_table
global p4_table
global p3_table
global p3_table_hh
global p1_tables

global stack

global start
extern kernel_main

[bits 32]

start:

    ; Move the multiboot info to 64 bit registers
    mov edi, ebx
    mov esi, eax

    ; Setup lower half of the stack
    mov esp, stack.top - KERNEL_VIRTUAL_ADDR

    ; Identity map the p4 table
    mov eax, p4_table - KERNEL_VIRTUAL_ADDR
    or eax, FLAGS
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 510 * 8], eax

    ; Identity map the p3 table
    mov eax, p3_table - KERNEL_VIRTUAL_ADDR
    or eax, FLAGS
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 0], eax

    ; Identity map the p2 table
    mov eax, p2_table - KERNEL_VIRTUAL_ADDR
    or eax, FLAGS
    mov dword [(p3_table - KERNEL_VIRTUAL_ADDR) + 0], eax

    ; Map the kernel into the higher half
    mov eax, p3_table_hh - KERNEL_VIRTUAL_ADDR
    or eax, FLAGS
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 511 * 8], eax

    ; Map the kernel into the higher half (second  level)
    mov eax, p2_table - KERNEL_VIRTUAL_ADDR
    or eax, FLAGS
    mov dword[(p3_table_hh - KERNEL_VIRTUAL_ADDR) + 510 * 8], eax

    ; Map 8MB of kernel memory  (2 page directories)
    mov ebx, 0
    mov eax, p1_tables - KERNEL_VIRTUAL_ADDR
    .map_pd_table:

        ; Map the page directory
        or eax, FLAGS
        mov dword[(p2_table - KERNEL_VIRTUAL_ADDR) + ebx * 8], eax
        add eax, PAGE_SIZE

        ; Loop
        inc ebx
        cmp ebx, PD_LOOP_LIMIT
        jne .map_pd_table

    ; Fill the page directory with the kernel page tables
    mov ecx, 0
    .map_p2_table:

        ; Map the page
        mov eax, PAGE_SIZE
        mul ecx
        or eax, FLAGS
        mov [(p1_tables - KERNEL_VIRTUAL_ADDR) + ecx * 8], eax

        ; Loop
        inc ecx
        cmp ecx, LOOP_LIMIT
        jne .map_p2_table

    ; Load the now setup p4
    mov eax, (p4_table - KERNEL_VIRTUAL_ADDR)
    mov cr3, eax

    ; Enable Physical Address Extension (PAE)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enable long mode in EFER
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging & write protection
    mov eax, cr0
    or eax, 1 << 31
    or eax, 1 << 16
    mov cr0, eax

    ; Load a basic GDT to be able to enter long mode
    lgdt [gdt64.pointer - KERNEL_VIRTUAL_ADDR]

    ; Jump to 64 bit mode using the kernel code selector
    jmp 0x8:(jump_to_higher_half - KERNEL_VIRTUAL_ADDR)

section .text

[bits 64]
jump_to_higher_half:

    ; Now in 64 bit mode so can access the higher half of the memory
    jmp kernel_entry + KERNEL_VIRTUAL_ADDR

kernel_entry:

    ; Reset the stack pointer
    mov rsp, stack.top

    ; Run the kernel
    call kernel_main

section .bss

align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p3_table_hh:
    resb 4096
p2_table:
    resb 4096
p1_tables:
    resb 8192


; The stack for the kernel
align 4096
stack:
    resb 16384
    .top:

; Create a bare minimum gdt that can be used to enter long mode
section .data
gdt64:
    dq  0
    dq (1 <<44) | (1 << 47) | (1 << 41) | (1 << 43) | (1 << 53)
    dq 0
    dq 0
    dq 0
    dq 0
    dq 0

.pointer:
    dw .pointer - gdt64 - 1             ; size
    dq gdt64 - KERNEL_VIRTUAL_ADDR      ; address