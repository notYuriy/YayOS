bits 64

YY_ExitProcess: equ 0
YY_ConsoleWrite: equ 2

section .data
msg: db "Hello, world!", 13, 10, 0
msglen: equ $ - msg

section .text
    global _start:
_start:
    mov rdi, msg
    mov rsi, msglen
    mov rax, YY_ConsoleWrite
    int 57h

    mov rax, YY_ExitProcess
    int 57h
