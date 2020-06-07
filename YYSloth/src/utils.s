bits 64

global getPageTable
global getFlags
global getCR2
global getFS
global getCS
global getGS
global getDS
global getES
global getSS

getPageTable:
    mov rax, cr3
    ret

getFlags:
    pushfq
    pop rax
    ret

getFS:
    mov rax, fs
    ret

getCS:
    mov rax, cs
    ret

getDS:
    mov rax, ds
    ret

getGS:
    mov rax, gs
    ret

getES:
    mov rax, es
    ret

getSS:
    mov rax, ss
    ret

getCR2:
    mov rax, cr2
    ret