; Credit: https://github.com/dreamos82/Dreamos64/

%define KERNEL_VIRTUAL_ADDR 0xFFFFFFFF80000000
section .multiboot.text
global start
extern callConstructors
extern kernelMain

[bits 32]

start:
    mov edi, ebx ; Address of multiboot structure
    mov esi, eax ; Magic number

    mov esp, stack.top - KERNEL_VIRTUAL_ADDR

    ; For now we are goin to use 2Mib pages
    ; We need only 3 table levels instead of 4
    mov eax, p3_table - KERNEL_VIRTUAL_ADDR; Copy p3_table address in eax
    or eax, 0b11        ; set writable and present bits to 1
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 0], eax   ; Copy eax content into the entry 0 of p4 table

    mov eax, p3_table_hh - KERNEL_VIRTUAL_ADDR
    or eax, 0b11
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 511 * 8], eax

    mov eax, p2_table - KERNEL_VIRTUAL_ADDR  ; Let's do it again, with p2_table
    or eax, 0b11       ; Set the writable and present bits
    mov dword [(p3_table - KERNEL_VIRTUAL_ADDR) + 0], eax   ; Copy eax content in the 0th entry of p3

    mov eax, p2_table - KERNEL_VIRTUAL_ADDR
    or eax, 0b11
    mov dword[(p3_table_hh - KERNEL_VIRTUAL_ADDR) + 510 * 8], eax

    ; Now let's prepare a loop...
    mov ecx, 0  ; Loop counter

    .map_p2_table:
        mov eax, 0x200000   ; Size of the page
        mul ecx             ; Multiply by counter
        or eax, 0b10000011 ; We set: huge page bit, writable and present

        ; Moving the computed value into p2_table entry defined by ecx * 8
        ; ecx is the counter, 8 is the size of a single entry
        mov [(p2_table - KERNEL_VIRTUAL_ADDR) + ecx * 8], eax

        inc ecx             ; Let's increase ecx
        cmp ecx, 512        ; have we reached 512 ?
                            ; each table is 4k size. Each entry is 8bytes
                            ; that is 512 entries in a table

        jne .map_p2_table   ; if ecx < 512 then loop

    ; This section is temporary, is here only to test the framebuffer features!
    ; Will be removed once the the memory management will be implemented
    mov eax, fbb_p2_table - KERNEL_VIRTUAL_ADDR
    or eax, 0b11
    mov dword [(p3_table - KERNEL_VIRTUAL_ADDR)+ 8 * 3], eax

    mov eax, 0xFD000000
    or eax, 0b10000011
    mov dword [(fbb_p2_table - KERNEL_VIRTUAL_ADDR) + 8 * 488], eax

    ; Now time to map the kernel in the higher half
    ; mov eax, hhk_p2_table
    ; or eax, 0b11
    ; mov dword [fbb_p2_table + 0], eax

    mov eax, 0x000000 ; This is the base address of the kernel
    or eax, 0b10000011
    mov dword [(fbb_p2_table - KERNEL_VIRTUAL_ADDR) + 0], eax

    mov eax, 0x200000
    or eax, 0b10000011
    mov dword [(fbb_p2_table - KERNEL_VIRTUAL_ADDR) + 8 * 1], eax ;  Multiply by 1 just to highlight the entry number

    ; All set... now we are nearly ready to enter into 64 bit
    ; Is possible to move into cr3 only from another register
    ; So let's move p4_table address into eax first
    ; then into cr3
    mov eax, (p4_table - KERNEL_VIRTUAL_ADDR)
    mov cr3, eax

    ; Now we can enable PAE
    ; To do it we need to modify cr4, so first let's copy it into eax
    ; we can't modify cr registers directly
    mov eax, cr4
    or eax, 1 << 5  ; Physical address extension bit
    mov cr4, eax

    ; Now set up the long mode bit
    mov ecx, 0xC0000080
    ; rdmsr is to read a a model specific register (msr)
    ; it copy the values of msr into eax
    rdmsr
    or eax, 1 << 8
    ; write back the value
    wrmsr

    ; Now is tiem to enable paging
    mov eax, cr0    ;cr0 contains the values we want to change
    or eax, 1 << 31 ; Paging bit
    or eax, 1 << 16 ; Write protect, cpu  can't write to read-only pages when
                    ; privilege level is 0
    mov cr0, eax    ; write back cr0

    ; load gdt
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

; This section is temporary to test the framebuffer
align 4096
fbb_p3_table:
    resb 4096
fbb_p2_table:
    resb 4096

stack:
    resb 16384
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

