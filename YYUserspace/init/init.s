bits 64

YY_ConsoleWrite: equ 2
YY_VirtualAlloc: equ 5
YY_VirtualFree: equ 6
YY_ExitProcess: equ 0
YY_VirtualFlagsWritable: equ 1 << 0

section .bss
stackBottom:
resb 4096
stackTop:

section .data
msg: db "Hello world from allocated memory", 13, 10, 0

section .text
    global _start
    
; rdi - src
; rsi - dst
strcpy:
    push rdi
    push rsi
.loop:
    mov al, byte [rdi]
    mov byte [rsi], al
    cmp al, 0
    je .done
    inc rdi
    inc rsi
    jmp .loop
.done:
    pop rsi
    pop rdi
    ret
    
; rdi - str
; out: rsi - len
strlen:
    push rdi
    xor rsi, rsi
.loop:
    mov al, byte [rdi]
    cmp al, 0
    je .done
    inc rsi
    inc rdi
    jmp .loop
.done:
    pop rdi
    ret
    
    
_start:
    mov rsp, stackTop
    
    mov rax, YY_VirtualAlloc
    mov rsi, 1 ; one 0x1000 sized page
    mov rdi, YY_VirtualFlagsWritable
    int 57h
    
    mov rdi, msg
    mov rsi, rax
    call strcpy
    
    mov rdi, rsi
    call strlen
    
    mov rax, YY_ConsoleWrite
    int 57h
    
    mov rax, YY_VirtualFree
    mov rsi, 1
    int 57h
    
    mov rax, YY_ExitProcess
    int 57h