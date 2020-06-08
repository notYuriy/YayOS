bits 64

YY_ExitProcess: equ 0
YY_DuplicateProcess: equ 1
YY_CheckProcessStatus: equ 8
YY_ConsoleWrite: equ 2

section .data
chldmsg: db "Message from the child", 13, 10
chldmsglen: equ $ - chldmsg
msg: db "Child Terminated", 13, 10
msglen: equ $ - msg
waitmsg: db "Waiting for child", 13, 10
waitmsglen: equ $ - waitmsg

section .text
    global _start
_start:
    mov rdi, chldmsg
    mov rsi, chldmsglen
    mov rax, YY_ConsoleWrite
    int 57h

    mov rax, YY_DuplicateProcess
    int 57h

    cmp rax, 0
    je .child
.parent:
    mov rdi, rax
.loop:
    mov rax, YY_CheckProcessStatus
    int 57h
    cmp rax, 1
    je .done
    jmp .loop
.done: 
    ; child terminated
    mov rdi, msg
    mov rsi, msglen
    mov rax, YY_ConsoleWrite
    int 57h

    mov rax, YY_ExitProcess
    int 57h

.child:
    mov rdi, chldmsg
    mov rsi, chldmsglen
    mov rax, YY_ConsoleWrite
    int 57h

    mov rax, YY_ExitProcess
    int 57h