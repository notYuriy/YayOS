bits 64

section .text

extern scheduleUsingFrame
extern timerEOI
extern zeroIntLevel
global schedulerYield
global schedulerIntHandler

schedulerIntHandler:
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    mov rax, cr3
    push rax
    mov rax, es
    push rax
    mov rax, ds
    push rax
    mov rax, gs
    push rax
    mov rax, fs
    push rax
    mov rdi, rsp
    call scheduleUsingFrame
    call timerEOI
    pop rax
    mov fs, rax
    pop rax
    mov gs, rax
    pop rax
    mov ds, rax
    pop rax
    mov es, rax
    pop rax
    mov rbx, cr3
    cmp rbx, rax
    je .next
    mov cr3, rax
.next:
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
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
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    mov rax, cr3
    push rax
    mov rax, es
    push rax
    mov rax, ds
    push rax
    mov rax, gs
    push rax
    mov rax, fs
    push rax
    mov rdi, rsp
    call scheduleUsingFrame
    call zeroIntLevel
    pop rax
    mov fs, rax
    pop rax
    mov gs, rax
    pop rax
    mov ds, rax
    pop rax
    mov es, rax
    pop rax
    mov rbx, cr3
    cmp rbx, rax
    je .next
    mov cr3, rax
.next:
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    sti
    iretq
.recoverPoint:
    pop rax
    ret