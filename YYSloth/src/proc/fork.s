bits 64

section .text

%include "proc/state.inc"

extern sysForkWithFrame
global YY_DuplicateProcess

YY_DuplicateProcess:
; idt frame begin
    mov rax, ss
    push rax
    mov rax, rsp
    add rax, 8
    push rax
    pushfq
    mov rax, cs
    push rax
    mov rax, .recoverPoint
    push rax
    PUSHREGS
    call sysForkWithFrame
    POPREGS
    iretq
.recoverPoint:
    ret
