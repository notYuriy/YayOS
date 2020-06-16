bits 64

extern YY_Main
global run


%macro DEFINE_SYSCALL 2
global %1
%1:
    mov rax, %2
    int 57h
    ret
%endmacro

section .bss
    stackbottom:
        resb 65536
    stacktop:

section .text
    global _start
_start:
    mov rsp, stacktop
    call YY_Main
    mov rdi, rax
    xor rax, rax
    int 57h

DEFINE_SYSCALL YY_ExitProcess, 0
DEFINE_SYSCALL YY_DuplicateProcess, 1
DEFINE_SYSCALL YY_ConsoleWrite, 2
DEFINE_SYSCALL YY_GetSystemInfo, 3
DEFINE_SYSCALL YY_Yield, 4
DEFINE_SYSCALL YY_VirtualAlloc, 5
DEFINE_SYSCALL YY_VirtualFree, 6
DEFINE_SYSCALL YY_QueryAPIInfo, 7
DEFINE_SYSCALL YY_GetProcessStatus, 8
DEFINE_SYSCALL YY_ExecuteBinary, 9
DEFINE_SYSCALL YY_OpenFile, 10
DEFINE_SYSCALL YY_ReadFile, 11
DEFINE_SYSCALL YY_WriteFile, 12
DEFINE_SYSCALL YY_GetFilePos, 13
DEFINE_SYSCALL YY_SetFilePos, 14
DEFINE_SYSCALL YY_CloseFile, 15
DEFINE_SYSCALL YY_ReadDirectory, 16

run:
    mov rax, 1
    int 57h
    cmp rax, -1
    je $
    cmp rax, 0
    jne .par
.chld:
    mov rax, 9
    int 57h
    mov rdi, 0xcafebabedeadbeef
    xor rax, rax
    int 57h
.par:
    mov rdi, rax
    mov rsi, rcx
.loop:
    mov rax, 8
    int 57h
    cmp rax, 1
    je .done
    mov rax, 4
    int 57h
    jmp .loop
.done:
    ret

