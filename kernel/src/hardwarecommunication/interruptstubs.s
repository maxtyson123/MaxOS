section .data
    IRQ_BASE equ 0x20
    interruptnumber db 0

section .text
    extern _ZN5MaxOS21hardwarecommunication16InterruptManager15HandleInterruptEhj

    %macro HandleException 1
    global _ZN5MaxOS21hardwarecommunication16InterruptManager19HandleException%1Ev
    _ZN5MaxOS21hardwarecommunication16InterruptManager19HandleException%1Ev:
        mov byte [interruptnumber], %1
        jmp int_bottom
    %endmacro

    %macro HandleInterruptRequest 1
    global _ZN5MaxOS21hardwarecommunication16InterruptManager26HandleInterruptRequest%1Ev
    _ZN5MaxOS21hardwarecommunication16InterruptManager26HandleInterruptRequest%1Ev:
        mov byte [interruptnumber], %1 + IRQ_BASE
        push 0
        jmp int_bottom
    %endmacro

HandleException 0x00
HandleException 0x01
HandleException 0x02
HandleException 0x03
HandleException 0x04
HandleException 0x05
HandleException 0x06
HandleException 0x07
HandleException 0x08
HandleException 0x09
HandleException 0x0A
HandleException 0x0B
HandleException 0x0C
HandleException 0x0D
HandleException 0x0E
HandleException 0x0F
HandleException 0x10
HandleException 0x11
HandleException 0x12
HandleException 0x13
HandleException 0x14
HandleException 0x15
HandleException 0x16
HandleException 0x17
HandleException 0x18
HandleException 0x19
HandleException 0x1A
HandleException 0x1B
HandleException 0x1C
HandleException 0x1D
HandleException 0x1E
HandleException 0x1F

HandleInterruptRequest 0x00
HandleInterruptRequest 0x01
HandleInterruptRequest 0x02
HandleInterruptRequest 0x03
HandleInterruptRequest 0x04
HandleInterruptRequest 0x05
HandleInterruptRequest 0x06
HandleInterruptRequest 0x07
HandleInterruptRequest 0x08
HandleInterruptRequest 0x09
HandleInterruptRequest 0x0A
HandleInterruptRequest 0x0B
HandleInterruptRequest 0x0C
HandleInterruptRequest 0x0D
HandleInterruptRequest 0x0E
HandleInterruptRequest 0x0F
HandleInterruptRequest 0x31
HandleInterruptRequest 0x80

int_bottom:
    push bp
    push di
    push si

    push dx
    push cx
    push bx
    push ax

    push sp
    push interruptnumber
    call _ZN5MaxOS21hardwarecommunication16InterruptManager15HandleInterruptEhj
    mov ax, sp

    pop ax
    pop bx
    pop cx
    pop dx

    pop si
    pop di
    pop bp

    add sp, 4

global _ZN5MaxOS21hardwarecommunication16InterruptManager15InterruptIgnoreEv
_ZN5MaxOS21hardwarecommunication16InterruptManager15InterruptIgnoreEv:
    iret
