; core_loader.s  (NASM)
global core_start
global core_boot_info

extern core_main

%define CORE_BOOT_INFO_OFFSET core_boot_info - core_start
%define CORE_BOOT_STACK      0
%define CORE_BOOT_P4_TABLE   8
%define CORE_BOOT_ID         16
%define CORE_BOOT_ACTIVATED  17
%define CORE_BOOT_GDT_64     24

%define KERNEL_VIRTUAL_ADDR 0xFFFFFFFF80000000

section .spinup
align 16

[bits 16]
core_start:

    ; Set up
    cli
    cld

    ; Load a minimal GDT
    lgdt [gdt32r]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Jump to 32 bit mode using the kernel code selector
    jmp 0x08:protected_mode_entry

[bits 32]
protected_mode_entry:

    ; Load the now setup p4
    mov eax, [0x8000 + CORE_BOOT_INFO_OFFSET + CORE_BOOT_P4_TABLE]
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

    ; Jump to 64 bit mode using the kernel code selector
    jmp 0x18:(core_jump_to_higher_half)

[bits 64]
core_jump_to_higher_half:

    ; Now in 64 bit mode so can access the higher half of the memory
    jmp core_entry + KERNEL_VIRTUAL_ADDR

core_entry:

    ; Load the temporary stack pointer
    mov rsp, [0x8000 + CORE_BOOT_INFO_OFFSET + CORE_BOOT_STACK]

    ; Mark the core as setup
    mov byte [0x8000 + CORE_BOOT_INFO_OFFSET + CORE_BOOT_ACTIVATED], 1

    lgdt [0x8000 + CORE_BOOT_INFO_OFFSET + CORE_BOOT_GDT_64]

    .loop: jmp .loop

   ; Now call it
   call core_main


; Boot info
align 8
core_boot_info:
    dq 0          ; stack
    dq 0          ; p4_table
    db 0          ; id
    db 0          ; activated
    dq 0          ; gdt_64_base

; Minimal GDT
align 8
gdt32:
    dq 0                      ; Null descriptor
    dq 0x00CF9A000000FFFF     ; 32-bit code segment
    dq 0x00CF92000000FFFF     ; 32-bit data segment
    dq 0x00AF9B000000FFFF     ; 64-bit code segment
    dq 0x00AF93000000FFFF     ; 64-bit data segment

gdt32_end:

; GDTR structure
align 8
gdt32r:
    dw gdt32_end - gdt32 - 1        ; Limit
    dq gdt32                        ; Base