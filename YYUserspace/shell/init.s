bits 64

extern YY_Main

global YY_ConsoleWrite

section .bss
    stackbottom:
        resb 65536
    stacktop:

section .text
    global _start
_start:
    mov rsp, stacktop
    call YY_Main
    xor rax, rax
    int 57h

YY_ConsoleWrite:
    mov rax, 2
    int 57h

