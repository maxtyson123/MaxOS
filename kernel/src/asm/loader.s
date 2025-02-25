%define MULTIBOOT_TAG_TYPE_END  0
%define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO 4
%define MULTIBOOT_TAG_TYPE_MMAP 6
%define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8
%define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR 21
%define MULTIBOOT_TAG_TYPE_ACPI_OLD 14
%define MULTIBOOT_TAG_TYPE_ACPI_NEW 15
struc multiboot_tag
    .type:  resd 1
    .size:  resd 1
endstruc

struc multiboot_tag_framebuffer
    .type:  resd 1
    .size:  resd 1
    .framebuffer_addr:  resq 1
    .framebuffer_pitch: resd 1
    .framebuffer_width: resd 1
    .framebuffer_height:    resd 1
    .framebuffer_bpp:   resb    1
    .framebuffer_type:  resb    1
    .reserved:  resb 1
endstruc


%define KERNEL_VIRTUAL_ADDR 0xFFFFFFFF80000000
%define PAGE_DIR_ENTRY_FLAGS 0b11
%define SMALL_PAGES 1
%define PRESENT_BIT 1
%define WRITE_BIT 0b10
%define HUGEPAGE_BIT 0b10000000

%if SMALL_PAGES == 1
%define s_page_size 0x1000 ; s_page_size is 4k
%define PAGE_TABLE_ENTRY WRITE_BIT | PRESENT_BIT ;PAGE_TABLE_ENTRY for 4k pages, huge page bit is left to 0
%define LOOP_LIMIT 1024
%define PD_LOOP_LIMIT 2
%elif SMALL_PAGES == 0
%define s_page_size 0x200000
%define PAGE_TABLE_ENTRY HUGEPAGE_BIT | WRITE_BIT | PRESENT_BIT ;PAGE_TABLE (pd table) entry for 2M pages, huge page bit is set.
%define LOOP_LIMIT 512
%endif

section .multiboot.text
global start
global p2_table
global p4_table
global p3_table
global p3_table_hh
%if SMALL_PAGES == 1
global pt_tables
%endif
global end_of_mapped_memory ;this variable will contain the virtual address of the last address mapped after bootstrap
global multiboot_framebuffer_data
global multiboot_mmap_data
global multiboot_basic_meminfo
global multiboot_acpi_info
global multiboot_tag_start
global multiboot_tag_end
global read_multiboot
global gdt64
global stack

; extern kernel_tss
extern kernelMain
extern callConstructors

[bits 32]

start:
    mov edi, ebx ; Address of multiboot structure
    mov esi, eax ; Magic number

    mov esp, stack.top - KERNEL_VIRTUAL_ADDR

    mov eax, p3_table - KERNEL_VIRTUAL_ADDR; Copy p3_table address in eax
    or eax, PRESENT_BIT | WRITE_BIT        ; set writable and present bits to 1
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 0], eax   ; Copy eax content into the entry 0 of p4 table

    mov eax, p3_table_hh - KERNEL_VIRTUAL_ADDR  ; This will contain the mapping of the kernel in the higher half
    or eax, PRESENT_BIT | WRITE_BIT
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 511 * 8], eax

    mov eax, p4_table - KERNEL_VIRTUAL_ADDR ; Mapping the PML4 into itself
    or eax, PRESENT_BIT | WRITE_BIT
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 510 * 8], eax

    mov eax, p2_table - KERNEL_VIRTUAL_ADDR  ; Let's do it again, with p2_table
    or eax, PRESENT_BIT | WRITE_BIT       ; Set the writable and present bits
    mov dword [(p3_table - KERNEL_VIRTUAL_ADDR) + 0], eax   ; Copy eax content in the 0th entry of p3

    mov eax, p2_table - KERNEL_VIRTUAL_ADDR
    or eax, PRESENT_BIT | WRITE_BIT
    mov dword[(p3_table_hh - KERNEL_VIRTUAL_ADDR) + 510 * 8], eax

    %if SMALL_PAGES == 1
    ; If we are using 4k pages we have an extra level of tables to map
    mov ebx, 0
    mov eax, pt_tables - KERNEL_VIRTUAL_ADDR
    .map_pd_table:
        or eax, PRESENT_BIT | WRITE_BIT
        mov dword[(p2_table - KERNEL_VIRTUAL_ADDR) + ebx * 8], eax
        add eax, 0x1000
        inc ebx
        cmp ebx, PD_LOOP_LIMIT
        jne .map_pd_table
    %endif
    ; Now let's prepare a loop...
    mov ecx, 0  ; Loop counter

    .map_p2_table:
        mov eax, s_page_size  ; Size of the page
        mul ecx             ; Multiply by counter
        or eax, PAGE_TABLE_ENTRY ; We set: huge page bit (if on 2M pages), writable and present

        ; Moving the computed value into p2_table entry defined by ecx * 8
        ; ecx is the counter, 8 is the size of a single entry
        %if SMALL_PAGES == 1
        mov [(pt_tables - KERNEL_VIRTUAL_ADDR) + ecx * 8], eax
        %elif SMALL_PAGES == 0
        mov [(p2_table - KERNEL_VIRTUAL_ADDR) + ecx * 8], eax
        %endif

        inc ecx             ; Let's increase ecx
        cmp ecx, LOOP_LIMIT        ; have we reached 512 ? (1024 for small pages)
                            ; When small pages is enabled:
                            ; each table is 4k size. Each entry is 8bytes
                            ; that is 512 entries in a table
                            ; when small pages enabled: two tables are adjacent in memory
                            ; they are mapped in the pdir during the map_pd_table cycle
                            ; this is why the loop is up to 1024

        jne .map_p2_table   ; if ecx < 512 then loop


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

    ; Now is time to enable paging
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

    %if SMALL_PAGES == 0
    mov qword[(end_of_mapped_memory - KERNEL_VIRTUAL_ADDR)], (511 << 39) | (510 << 30) | (511 << 21)
    %elif SMALL_PAGES == 1
    mov qword[(end_of_mapped_memory - KERNEL_VIRTUAL_ADDR)], (511 << 39) | (510 << 30) | ((PD_LOOP_LIMIT-1) << 21) | (511 << 12)
    %endif
    ; update segment selectors
    mov ax, 0x10
    mov ss, ax  ; Stack segment selector
    mov ds, ax  ; data segment register
    mov es, ax  ; extra segment register
    mov fs, ax  ; extra segment register
    mov gs, ax  ; extra segment register

    lea rax, [rdi+8]

    ;.bss section should be already 0  at least on unix and windows systems
    ;no need to initialize
    mov [multiboot_tag_start], rax

read_multiboot:
    ;Check if the tag is needed by the kernel, if yes store its address
    cmp dword [rax + multiboot_tag.type], MULTIBOOT_TAG_TYPE_FRAMEBUFFER
    je .parse_fb_data
    cmp dword [rax + multiboot_tag.type], MULTIBOOT_TAG_TYPE_MMAP
    je .mmap_tag_item
    cmp dword [rax + multiboot_tag.type], MULTIBOOT_TAG_TYPE_BASIC_MEMINFO
    je .meminfo_tag_item
    cmp dword [rax + multiboot_tag.type], MULTIBOOT_TAG_TYPE_ACPI_OLD
    je .acpi_item
    cmp dword [rax + multiboot_tag.type], MULTIBOOT_TAG_TYPE_ACPI_NEW
    je .acpi_item
    jmp .item_not_needed
    .parse_fb_data:
        mov [multiboot_framebuffer_data], rax
    ; Here mapping the first 4mb(2mb using 4k pages)  of framebuffer
    ; The rest of the initialization will be done in the _init_basic_system functin
    %if SMALL_PAGES == 0
        mov rbx, [(rax + multiboot_tag_framebuffer.framebuffer_addr)]
        or rbx, PAGE_TABLE_ENTRY
        mov qword [(p2_table - KERNEL_VIRTUAL_ADDR) + 8 * 488], rbx
        add rbx, s_page_size
        or rbx, PAGE_TABLE_ENTRY
        mov qword [(p2_table - KERNEL_VIRTUAL_ADDR) + 8 * 489], rbx
    %else
        mov rcx, 0
        mov rbx, fbb_pt_tables - KERNEL_VIRTUAL_ADDR
        or rbx, PRESENT_BIT | WRITE_BIT
        mov qword [(p2_table) + 8 * 488], rbx
        mov rbx, [rax + multiboot_tag_framebuffer.framebuffer_addr]
        .map_fb:
            or  rbx, PAGE_TABLE_ENTRY
            mov qword [(fbb_pt_tables) + 8 * rcx], rbx
            add rbx, s_page_size
            inc rcx
            cmp rcx, 512
            jne .map_fb
        ;mov qword [p2_table + 8 * 488],
    %endif
        jmp .item_not_needed
    .mmap_tag_item:
        mov [multiboot_mmap_data], rax
        jmp .item_not_needed
    .acpi_item:
        mov [multiboot_acpi_info], rax
        jmp .item_not_needed
    .meminfo_tag_item:
        mov [multiboot_basic_meminfo], rax
    .item_not_needed:
        mov ebx, dword [rax + multiboot_tag.size]
        ;Next tag is at current_tag_address + current_tag size
        ;lea rax, [rax + rbx + 7]
        add rax, rbx
        ;Padded with 0 until the first byte aligned with 8bytes
        add rax, 7
        and rax, ~7
        ;Check if the tag is the end tag
        ;Type: 0 Size: 8
        ;multiboot_tag.type == 0?
        cmp dword [rax + multiboot_tag.type], MULTIBOOT_TAG_TYPE_END
        jne read_multiboot
        ; && multiboot_tag.size == 8?
        cmp dword [rax + multiboot_tag.size], 8
        jne read_multiboot
        add rax, multiboot_tag.size
        mov qword [multiboot_tag_end], rax

    mov rax, higher_half
    jmp rax

higher_half:
    ; Far jump to long mode
    mov rsp, stack.top
    lgdt [gdt64.pointer]

    ; The two lines below are needed to un map the kernel in the lower half
    mov eax, 0x0
    mov dword [(p4_table - KERNEL_VIRTUAL_ADDR) + 0], eax

    mov rax, cr3
    mov cr3, rax
    call callConstructors
    call kernelMain

section .bss

align 4096
p4_table: ;PML4
    resb 4096
p3_table: ;PDPR
    resb 4096
p3_table_hh: ;PDPR
    resb 4096
p2_table: ;PDP
    resb 4096

%if SMALL_PAGES == 1
; if SMALL_PAGES is defined it means we are using 4k pages
; For now the first 8mb will be mapped for the kernel.
; We reserve 8192 bytes, because we are going to fill 2 page tables
pt_tables:
    resb 8192
fbb_pt_tables:
    resb 8192
%endif


; This section will be used to get the multiboot info
align 4096
end_of_mapped_memory:
    resq 1
multiboot_tag_end:
    resq 1
multiboot_tag_start:
    resq 1
multiboot_framebuffer_data:
    resb 8
multiboot_mmap_data:
    resb 8
multiboot_basic_meminfo:
    resb 8
multiboot_acpi_info:
    resb 8
stack:
    resb 16384
    .top:

section .data
; gdt table needs at least 3 entries:
;     the first entry is always null
;     the other two are data segment and code segment.
;     the last two are data and code segments for user mode.
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

.pointer:
    dw .pointer - gdt64 - 1
    dq gdt64
.pointer_low:
    dw .pointer - gdt64 - 1
    dq gdt64 - KERNEL_VIRTUAL_ADDR

