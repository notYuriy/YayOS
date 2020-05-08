bits 64

global syscallInit
section .text

syscallInit:
    push rax
    push rdx
    push rcx
    mov rax, syscallHandler
    mov rbx, syscallHandler
    shl rbx, 32
    mov ecx, 0xC0000082
    wrmsr
    xor rax, rax
    xor rbx, rbx
    mov ecx, 0xC0000084
    wrmsr
    xor rax, rax
    mov rbx, (0x13 << 16) | (0x8)
    mov ecx, 0xC0000081
    pop rcx
    pop rdx
    pop rax
    ret

syscallHandler:
    

