bits 64

section .text
extern syscallHandlerTable
global syscallHandler

%include "proc/state.inc"

syscallHandler:
    push rax
    mov ax, 0x10
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    pop rax
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
    cmp rax, 1000
    jae .invalid
    mov rbx, syscallHandlerTable
    mov rbx, qword [rbx + 8 * rax]
    mov rax, rbx
    cmp rax, 0
    je .invalid
    call rax
    jmp .done
.invalid:
    int3
    mov rax, -1
.done:
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
    iretq

