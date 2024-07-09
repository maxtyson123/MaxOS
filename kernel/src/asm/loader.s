; Credit: https://github.com/dreamos82/Dreamos64/

; DEFINES
%define KERNEL_VIRTUAL_ADDR 0xFFFFFFFF80000000
%define KERNEL_TABLES 2
%define PRESENT_BIT 1
%define WRITE_BIT 0b10
%define PAGE_SIZE 0x1000

; VARS
section .multiboot.text
global start
global p4_table
extern callConstructors
extern kernelMain

[bits 32]

start:

    ; Put the multiboot header and magic number into the parameters of the kernelMain
    mov edi, ebx
    mov esi, eax

    ; Move the stack to the higher half
    mov esp, stack.top - KERNEL_VIRTUAL_ADDR

    ; pm4l -> pdp -> pd -> pt -> page

    ; = Configure the first entry in the P4 table to point the the P3 table =
    ; * p4_table =  pm4l, p3_table = pdp *
    ; First we change the address to the higher half
    ; Then we set the writable and present bits
    ; Then we copy the address into the first entry of the p4_table
    mov eax, p3_table - KERNEL_VIRTUAL_ADDR
    or eax, PRESENT_BIT | WRITE_BIT
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 0], eax

    ; =  Configure the the last entry in the P4 table to point the the P3 table =
    ; * p4_table =  pm4l, p3_table = pdp *
    ; First we change the address to the higher half
    ; Then we set the writable and present bits
    ; Then we copy the address into the last entry of the p4_table
    mov eax, p3_table_hh - KERNEL_VIRTUAL_ADDR
    or eax, PRESENT_BIT | WRITE_BIT
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 511 * 8], eax

    ; = Configure p4 table to point to map itself =
    ; * p4_table =  pm4l *
    ; First we change the address to the higher half
    ; Then we set the writable and present bits
    ; Then we copy the address into the last entry of the p4_table
    mov eax, p4_table - KERNEL_VIRTUAL_ADDR
    or eax, PRESENT_BIT | WRITE_BIT
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 510 * 8], eax

    ; =  Configure the first entry in the P3 table to point the the P2 table =
    ; * p3_table = pdp, p2_table = pd *
    ; First we change the address to the higher half
    ; Then we set the writable and present bits
    ; Then we copy the address into the first entry of the p3_table
    mov eax, p2_table - KERNEL_VIRTUAL_ADDR
    or eax, PRESENT_BIT | WRITE_BIT
    mov dword [(p3_table - KERNEL_VIRTUAL_ADDR) + 0], eax

    ; =  Configure the last entry in the P3 table to point the the P2 table =
    ; * p3_table = pdp, p2_table = pd *
    ; First we change the address to the higher half
    ; Then we set the writable and present bits
    ; Then we copy the address into the last entry of the p3_table
    mov eax, p2_table - KERNEL_VIRTUAL_ADDR
    or eax, PRESENT_BIT | WRITE_BIT
    mov dword[(p3_table_hh - KERNEL_VIRTUAL_ADDR) + 510 * 8], eax


    ; = Loop through 2 page tables and map them =
    mov ebx, 0
    mov eax, pt_tables - KERNEL_VIRTUAL_ADDR
    .map_pd_table:
        or eax, PRESENT_BIT | WRITE_BIT
        mov dword[(p2_table - KERNEL_VIRTUAL_ADDR) + ebx * 8], eax
        add eax, 0x1000
        inc ebx
        cmp ebx, KERNEL_TABLES
        jne .map_pd_table

    ; Now let's prepare a loop...
    mov ecx, 0  ; Loop counter

    ; = Loop through all the entries in the P2 table and set them to point to a 4KiB page =
    ; First we set a counter for the loop
    ; Then we set the size of the page to 2MB
    ; Then we multiply the size of the page by the counter
    ; Then we set the huge page bit, writable and present
    ; Then we copy the address into the entry in the p2_table
    ; After that we increment the counter and check if we have reached the end of the table
    .map_p2_table:
        mov eax, PAGE_SIZE
        mul ecx
        or eax, PRESENT_BIT | WRITE_BIT

        mov [(pt_tables - KERNEL_VIRTUAL_ADDR) + ecx * 8], eax

        inc ecx
        cmp ecx, 1024

        jne .map_p2_table

    ; = Load the P4 table into the cr3 register =
    ; CR3 is the control register 3, it contains the address of the P4 table, however it can't be loaded directly
    ; So we need to load it into eax and then copy it into cr3
    mov eax, (p4_table - KERNEL_VIRTUAL_ADDR)
    mov cr3, eax

    ; = Enable PAE (Physical Address Extension) =
    ; First we copy the value of cr4 into eax so we can change it
    ; Then we set the Physical Address Extension bit
    ; Then we copy the value back into cr4
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; = Enable Long Mode =
    ; First we use the rdmsr instruction to read the value of the msr 0xC0000080 into eax
    ; Then we set the Long Mode Enable bit
    ; Then we use the wrmsr instruction to write the value of eax into the msr 0xC0000080
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; = Enable paging =
    ; First we copy the value of cr0 into eax so we can change it
    ; Then we set the Paging bit
    ; Then we set the Write Protect bit this is to prevent the kernel from writing to read only pages
    mov eax, cr0
    or eax, 1 << 31
    or eax, 1 << 16
    mov cr0, eax

    ; = Jump to the higher half =
    ; First we load the address of the gdt into the gdtr
    ; Then we set the code segment to 0x8
    ; Then we jump to the higher half
    lgdt [gdt64.pointer_low - KERNEL_VIRTUAL_ADDR]
    jmp (0x8):(kernel_jumper - KERNEL_VIRTUAL_ADDR)
    bits 64

section .text
kernel_jumper:
    bits 64

    ; update segment selectors
    mov ax, 0x10
    mov ss, ax  ; Stack segment selector
    mov ds, ax  ; data segment register
    mov es, ax  ; extra segment register
    mov fs, ax  ; extra segment register
    mov gs, ax  ; extra segment register

    mov rax, higher_half
    jmp rax

higher_half:
    ; Far jump to long mode
    mov edx, 0xDEADBEEF

    mov rsp, stack.top

    lgdt [gdt64.pointer]

    ; Run the kernel
    call callConstructors
    call kernelMain

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
pt_tables:
    resb 8192   ; 2 tables for the kernel
stack:
    resb 32768 ; 16 KiB stack
    .top:

section .rodata

; gdt table needs at least 3 entries:
;     the first entry is always null
;     the other two are data segment and code segment.
gdt64:
    dq  0	;first entry = 0
    .code equ $ - gdt64
        ; set the following values:
        ; descriptor type: bit 44 has to be 1 for code and data segments
        ; present: bit 47 has to be  1 if the entry is valid
        ; read/write: bit 41 1 means that is readable
        ; executable: bit 43 it has to be 1 for code segments
        ; 64bit: bit 53 1 if this is a 64bit gdt
        dq (1 <<44) | (1 << 47) | (1 << 41) | (1 << 43) | (1 << 53)  ;second entry=code=8
    .data equ $ - gdt64
        dq (1 << 44) | (1 << 47) | (1 << 41)	;third entry = data = 10

.pointer:
    dw .pointer - gdt64 - 1
    dq gdt64
.pointer_low:
    dw .pointer - gdt64 - 1
    dq gdt64 - KERNEL_VIRTUAL_ADDR

