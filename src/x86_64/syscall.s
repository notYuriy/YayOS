bits 64

global syscallHandler
section .text

syscallStackTopOffset: equ 728
savedUserRspOffset: equ 736

syscallHandler:
    sti
    swapgs
    mov qword [gs:savedUserRspOffset], rsp
    mov rsp, qword [gs:syscallStackTopOffset]
    int3
