bits 64

global syscallHandler
extern syscallTable
section .text

syscallStackTopOffset: equ 728
savedUserRspOffset: equ 736

syscallHandler:
    swapgs
    mov qword [gs:savedUserRspOffset], rsp
    mov rsp, qword [gs:syscallStackTopOffset]
    sti
    push rbx
    push rbp
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    mov rbx, ds
    push rbx
    mov rbx, es
    push rbx
    mov rbx, gs
    push rbx
    mov rbx, fs
    push rbx
    mov bx, 0x10
    mov ds, bx
    mov es, bx
    mov gs, bx
    mov fs, bx

    cmp rax, 1000
    jg .invalidSyscall
    mov rbx, syscallTable
    shl rax, 3
    add rbx, rax
    mov rbx, [rbx]
    cmp rbx, 0
    je .invalidSyscall
    call rbx
    jmp .recovery
.invalidSyscall:
    mov rax, -38


.recovery:
    pop rbx
    mov fs, bx
    pop rbx
    mov gs, bx
    pop rbx
    mov es, bx
    pop rbx
    mov ds, bx
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbp
    pop rbx
    cli
    mov rsp, qword [gs:savedUserRspOffset]
    swapgs
    o64 sysret
    
