bits 64

section .text
global loadTSS

loadTSS:
    push rax
    mov ax, 0x28
    ltr ax
    pop rax
    ret