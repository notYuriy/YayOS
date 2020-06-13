bits 64

YY_ExitProcess: equ 0
YY_DuplicateProcess: equ 1
YY_CheckProcessStatus: equ 8
YY_ConsoleWrite: equ 2
YY_Yield: equ 4
YY_ExecuteBinary: equ 9

section .data
msg: db "Child Terminated", 13, 10
msglen: equ $ - msg
path: db "Y:\Binaries\helloworld", 0
args: dq 0

section .text
    global _start
_start:

    mov rax, YY_DuplicateProcess
    int 57h

    cmp rax, 0
    je .child

    mov rdi, rax
.loop:
    mov rax, YY_CheckProcessStatus
    int 57h

    cmp rax, 1
    je .done

    mov rax, YY_Yield
    int 57h
    
    jmp .loop
.done:

    mov rax, YY_ExitProcess
    int 57h

.child:
    mov rdi, path
    xor rsi, rsi
    mov rdx, args

    mov rax, YY_ExecuteBinary
    int 57h