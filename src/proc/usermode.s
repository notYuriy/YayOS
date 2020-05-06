bits 64

global jumpToUserMode

section .text

; arg1 - rdi - entryPoint
; arg2 - rsi - allocatedStack
jumpToUserMode:
    mov rax, 0x23 ; user data + RPL
    push rax
    push rsi
    ; probaby need
    ; to change this in the future
    ; because it is a clear
    ; vulnerability to leak
    ; kernel flags
    pushfq
    mov rax, 0x1b
    push rax
    push rdi
    xor rax, rax
    iretq


