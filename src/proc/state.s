bits 64

section .text

global extendedRegsLoadFromFpu
global extendedRegsSaveToFpu

extendedRegsLoadFromFpu:
    push rdx
    push rax
    mov rdx, 0xFFFFFFFFFFFFFFFF
    mov rax, 0xFFFFFFFFFFFFFFFF
    fxsave [rdi]
    pop rax
    pop rdx
    ret

extendedRegsSaveToFpu:
    push rdx
    push rax
    mov rdx, 0xFFFFFFFFFFFFFFFF
    mov rax, 0xFFFFFFFFFFFFFFFF
    fxrstor [rdi]
    pop rax
    pop rdx
    ret
