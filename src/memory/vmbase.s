section .text
bits 64

global vmbaseLoadP4
global vmbaseInvalidateCache

vmbaseLoadP4:
    mov cr3, rdi
    ret

vmbaseInvalidateCache:
    invlpg [rdi]
    ret