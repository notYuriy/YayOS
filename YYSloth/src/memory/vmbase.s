section .text
bits 64

global vmbaseLoadP4
global vmbaseInvalidateCache
global vmbaseInvalidateAll

vmbaseLoadP4:
    mov cr3, rdi
    ret

vmbaseInvalidateCache:
    invlpg [rdi]
    ret

vmbaseInvalidateAll:
    push rax
    mov rax, cr3
    mov cr3, rax
    pop rax
    ret
