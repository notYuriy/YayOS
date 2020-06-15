bits 64

YY_ExitProcess: equ 0
YY_ConsoleWrite: equ 2
YY_OpenFile: equ 10
YY_ReadFile: equ 11
YY_WriteFile: equ 12
YY_CloseFile: equ 15

section .data
buf: resb 56
term: db 13, 10
path: db "Y:\test.txt", 0
serialpath: db "D:\COM1", 0

section .text
    global _start:
_start:
    mov rdi, path
    xor rsi, rsi
    mov rax, YY_OpenFile
    int 57h
    mov r9, rax

    mov rdi, rax
    mov rsi, buf
    mov rdx, 56
    mov rax, YY_ReadFile
    int 57h

    mov rdi, serialpath
    mov rsi, 1
    mov rax, YY_OpenFile
    int 57h
    mov r10, rax

    mov rdi, rax
    mov rsi, buf
    mov rdx, 58
    mov rax, YY_WriteFile
    int 57h

    ; waiting for byte from serial port
.wait:
    mov rdi, r10
    mov rsi, buf
    mov rdx, 1
    mov rax, YY_ReadFile
    int 57h

    cmp rax, 0
    je .wait

    mov rdi, r9
    mov rax, YY_CloseFile
    int 57h

    mov rdi, r10
    mov rax, YY_CloseFile
    int 57h

    mov rax, YY_ExitProcess
    int 57h
