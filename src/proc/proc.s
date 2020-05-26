bits 64

section .text

%include "include/proc/state.inc"

extern scheduleUsingFrame
extern timerEOI
extern zeroIntLevel
global schedulerYield
global schedulerIntHandler

schedulerIntHandler:
    PUSHREGS
    call scheduleUsingFrame
    call timerEOI
    POPREGS
    iretq

schedulerYield:
    cli
    push rax
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
    xor rax, rax
    PUSHREGS
    call scheduleUsingFrame
    call zeroIntLevel
    POPREGS
    sti
    iretq
.recoverPoint:
    pop rax
    ret