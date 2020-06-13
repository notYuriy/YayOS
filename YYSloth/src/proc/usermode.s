bits 64

global jumpToUserMode

section .text

; arg1 - rdi - entryPoint
; arg2 - rsi - argc
; arg3 - rdx - argv
jumpToUserMode:
    mov rax, 0x23 ; user data + RPL
    push rax
    xor rax, rax
    push rax
    ; probably need
    ; to change this in the future
    ; because it is a clear
    ; vulnerability to leak
    ; kernel flags
    pushfq
    mov rax, 0x1b
    push rax
    push rdi
    mov rax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    xor rax, rax
    mov rdi, rsi
    mov rsi, rdx
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rbp, rbp
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
    iretq


