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
    mov rbx, syscallTable
    call [rbx + rax * 8]
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
    
