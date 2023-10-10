

.set IRQ_BASE, 0x20

.section .text

.extern _ZN5maxOS21hardwarecommunication16InterruptManager15HandleInterruptEhj


.macro HandleException num
.global _ZN5maxOS21hardwarecommunication16InterruptManager19HandleException\num\()Ev
_ZN5maxOS21hardwarecommunication16InterruptManager19HandleException\num\()Ev:
    movb $\num, (interruptnumber)
    #For an exception , the processor pushes an error value automatically
    jmp int_bottom
.endm


.macro HandleInterruptRequest num
.global _ZN5maxOS21hardwarecommunication16InterruptManager26HandleInterruptRequest\num\()Ev
_ZN5maxOS21hardwarecommunication16InterruptManager26HandleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interruptnumber)
    #Push 0  for the error
    pushl $0
    jmp int_bottom
.endm


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

    # Push Values From CPUState (multitasking.h)
    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax


    # Invoke C++ handlers
    pushl %esp
    push (interruptnumber)
    call _ZN5maxOS21hardwarecommunication16InterruptManager15HandleInterruptEhj

    # Switch the stack
    mov %eax, %esp

    # (In reverse ofc) Pop Values From CPUState (multitasking.h)
    popl %eax
    popl %ebx
    popl %ecx
    popl %edx

    popl %esi
    popl %edi
    popl %ebp

    add $4, %esp

.global _ZN5maxOS21hardwarecommunication16InterruptManager15InterruptIgnoreEv
_ZN5maxOS21hardwarecommunication16InterruptManager15InterruptIgnoreEv:

    iret


.data
    interruptnumber: .byte 0
