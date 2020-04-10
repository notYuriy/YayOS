bits 64

global spinlockLock
global spinlockTrylock
global spinlockUnlock
extern procYield

spinlockLock:
    push rax
    push rbx
.wait:
    mov rax, 0
    mov rbx, 1
    cmpxchg qword [rdi], rbx
    jz .done
    pause
    call procYield
    jmp .wait
.done:
    pop rbx
    pop rax
    ret

spinlockUnlock:
    push rax
    push rbx
    mov rax, 1
    mov rbx, 0
    cmpxchg qword [rdi], rbx
    pop rbx
    pop rax
    ret

spinlockTrylock:
    push rbx
.wait:
    mov rax, 0
    mov rbx, 1
    cmpxchg qword [rdi], rbx
    jnz .locked
    pop rbx
    mov rax, 1
    ret
.locked:
    pop rbx
    mov rax, 0
    ret