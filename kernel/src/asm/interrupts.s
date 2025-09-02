; Credit https://github.com/dreamos82/Dreamos64

[bits  64]
[extern _ZN5MaxOS21hardwarecommunication16InterruptManager15HandleInterruptEPNS_6system12cpu_status_tE]

%macro HandleException 1
[global _ZN5MaxOS21hardwarecommunication16InterruptManager19HandleException%1Ev]
_ZN5MaxOS21hardwarecommunication16InterruptManager19HandleException%1Ev:
    ; When this macro is called the status registers are already on the stack
    push 0	; since we have no error code, to keep things consistent we push a default EC of 0
    push %1 ; pushing the interrupt number for easier identification by the handler
    save_context ; Now we can save the general purpose registers
    mov rdi, rsp    ; Let's set the current stack pointer as a parameter of the interrupts_handler
    cld ; Clear the direction flag
    call _ZN5MaxOS21hardwarecommunication16InterruptManager15HandleInterruptEPNS_6system12cpu_status_tE ; Now we call the interrupt handler
    mov rsp, rax    ; use the returned context
    restore_context ; We served the interrupt let's restore the previous context
    add rsp, 16 ; We can discard the interrupt number and the error code
    iretq ; Now we can return from the interrupt
%endmacro

%macro HandleInterruptRequest 1
[global _ZN5MaxOS21hardwarecommunication16InterruptManager26HandleInterruptRequest%1Ev]
_ZN5MaxOS21hardwarecommunication16InterruptManager26HandleInterruptRequest%1Ev:
    ; When this macro is called the status registers are already on the stack
    push 0	; since we have no error code, to keep things consistent we push a default EC of 0
    push (%1 + 0x20) ; pushing the interrupt number for easier identification by the handler
    save_context ; Now we can save the general purpose registers
    mov rdi, rsp    ; Let's set the current stack pointer as a parameter of the interrupts_handler
    cld ; Clear the direction flag
    call _ZN5MaxOS21hardwarecommunication16InterruptManager15HandleInterruptEPNS_6system12cpu_status_tE ; Now we call the interrupt handler
    mov rsp, rax    ; use the returned context
    restore_context ; We served the interrupt let's restore the previous context
    add rsp, 16 ; We can discard the interrupt number and the error code
    iretq ; Now we can return from the interrupt
%endmacro

%macro HandleInterruptError 1
[global _ZN5MaxOS21hardwarecommunication16InterruptManager24HandleInterruptError%1Ev]
_ZN5MaxOS21hardwarecommunication16InterruptManager24HandleInterruptError%1Ev:
    push %1 ; In this case the error code is already present on the stack
    save_context
    mov rdi, rsp
    cld
    call _ZN5MaxOS21hardwarecommunication16InterruptManager15HandleInterruptEPNS_6system12cpu_status_tE
    mov rsp, rax    ; use the returned context
    restore_context
    add rsp, 16
    iretq
%endmacro

%macro save_context 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro restore_context 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

; Exception handlers
HandleException 0x00
HandleException 0x01
HandleException 0x02
HandleException 0x03
HandleException 0x04
HandleException 0x05
HandleException 0x06
HandleException 0x07
HandleInterruptError 0x08
HandleException 0x09
HandleInterruptError 0x0A
HandleInterruptError 0x0B
HandleInterruptError 0x0C
HandleInterruptError 0x0D
HandleInterruptError 0x0E
HandleException 0x0F
HandleException 0x10
HandleInterruptError 0x11
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

; Hardware interrupt handlers
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
HandleInterruptRequest 0x60
