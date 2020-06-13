bits 64

YY_ExitProcess: equ 0
YY_ConsoleWrite: equ 2
YY_OpenFile: equ 10
YY_ReadFile: equ 11

section .data
buf: resb 56
term: db 13, 10
path: db "Y:\test.txt", 0

section .text
    global _start:
_start:
    mov rdi, path
    xor rsi, rsi
    mov rax, YY_OpenFile
    int 57h

    mov rdi, rax
    mov rsi, buf
    mov rdx, 56
    mov rax, YY_ReadFile
    int 57h

    mov rdi, buf
    mov rsi, 58
    mov rax, YY_ConsoleWrite
    int 57h

    mov rax, YY_ExitProcess
    int 57h
