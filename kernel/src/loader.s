.section .mb2_hdr

; // multiboot2 header: magic number, mode, length, checksum
mb2_hdr_begin:
.long 0xE85250D6
.long 0
.long (mb2_hdr_end - mb2_hdr_begin)
.long -(0xE85250D6 + (mb2_hdr_end - mb2_hdr_begin))

; // framebuffer tag: type = 5
mb2_framebuffer_req:
    .short 5
    .short 1
    .long (mb2_framebuffer_end - mb2_framebuffer_req)
    ; // preferred width, height, bpp.
    ; // leave as zero to indicate "don't care"
    .long 0
    .long 0
    .long 0
mb2_framebuffer_end:

; // the end tag: type = 0, size = 8
.long 0
.long 8
mb2_hdr_end:

; // Base address of the boot stack
.section .data
boot_stack_base:
    .byte 0x1000

; // Back up the address of the multiboot structure as the ebx register may be overwritten
.section .mb_text
global _start
global gdt64
extern kernelMain

; // Function to start the kernel
_start:

   ; // Back up the address of the multiboot structure & magic number
    mov %ebx, %edi
    mov %eax, %esi

    ; // setup a stack, and reset flags
    mov $(boot_stack_base + 0x1000), %esp
    pushl $0x2
    popf

    ; // Setup the Global Descriptor Table in 64-bit mode
    lgdt gdt64

/* set up page tables for a higher half kernel */
/* don't forget to identity map all of physical memory */

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

    ; // set up the GDT
    call callConstructors

    # now we're in compatability mode,
    # after a long-jump to a 64-bit CS we'll be
    # in long-mode proper.
    push $gdt_64bit_cs_selector
    push $target_function
    lret

; // Setup the Global Descriptor Table in 64-bit mode
gdt64:
    dq  0	;first entry = 0
    .code equ $ - gdt64
        ; equ tells the compiler to set the address of the variable at given address ($ - gdt64). $ is the current position.
        ; set the following values:
        ; descriptor type: bit 44 has to be 1 for code and data segments
        ; present: bit 47 has to be  1 if the entry is valid
        ; read/write: bit 41 1 means that is readable
        ; executable: bit 43 it has to be 1 for code segments
        ; 64bit: bit 53 1 if this is a 64bit gdt
        dq (1 <<44) | (1 << 47) | (1 << 41) | (1 << 43) | (1 << 53)  ;second entry=code=0x8
    .data equ $ - gdt64
        dq (1 << 44) | (1 << 47) | (1 << 41)	;third entry = data = 0x10
    .ucode equ $ - gdt64
        dq (1 <<44) | (1 << 47) | (1 << 41) | (1 << 43) | (1 << 53) | (3 << 45) ;fourth entry=code=0x18
    .udata equ $ - gdt64
        dq (1 << 44) | (1 << 47) | (1 << 41) | (3 << 45)	;fifth entry = data = 0x20
    .tss_low equ $ - gdt64 ;sixth entry placeholder for TSS entry lower part
        dq 0
    .tss_high equ $ - gdt64 ; seventh entry placeholder for TSS entry higher part
        dq 0
