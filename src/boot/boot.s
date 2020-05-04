; See LICENSE-MIT file for license

global start
extern kmain
global p2_table

KERNEL_MAPPING_BASE equ 0xffff800000000000

section .inittext
bits 32
start:
    cli
    mov esi, ebx
    mov esp, stack_top - KERNEL_MAPPING_BASE  ; set up stack
    mov edi, ebx

    call check_multiboot
    call check_cpuid
    call check_long_mode

    call setup_page_tables
    call enable_paging

    call check_sse
    call enable_sse

    lgdt [gdt64.pointer_low - KERNEL_MAPPING_BASE]
    ; update selectors
    mov ax, gdt64.kernel_data
    mov ss, ax
    mov ds, ax
    mov es, ax

    jmp gdt64.kernel_code:prestart64

; Prints `ERR: ` and the given error code to screen and hangs.
; parameter: error code (in ascii) in al
error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt


check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "0"
    jmp error

check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    cpuid                  ; get highest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

setup_page_tables:
    mov eax, p3_table - KERNEL_MAPPING_BASE
    or eax, 0b11        ; present + writable
    mov [p4_table - KERNEL_MAPPING_BASE], eax
    mov [p4_table - KERNEL_MAPPING_BASE + 256 * 8], eax
    mov eax, p4_table - KERNEL_MAPPING_BASE
    or eax, 0b11
    mov [p4_table - KERNEL_MAPPING_BASE + 511 * 8], eax

    mov eax, p2_table - KERNEL_MAPPING_BASE
    or eax, 0b11        ; present + writable
    mov [p3_table - KERNEL_MAPPING_BASE], eax

    mov eax, 0; current address
    mov ebx, p1_tables - KERNEL_MAPPING_BASE ; current p1 table offset
    mov ecx, 0 ; current p2 table index
.next_p2_entry:
    cmp ecx, 32
    je .done
    mov edx, ebx
    or edx, 0b11 ; present + writable
    mov [p2_table - KERNEL_MAPPING_BASE + 8 * ecx], edx
    call map_p2_entry ; this will automatically add to eax and ebx
    inc ecx
    jmp .next_p2_entry
.done:
    ret

; eax - start address
; ebx - p1 entry address
map_p2_entry:
    push ecx
    mov ecx, 0
.next_p1_entry:
    cmp ecx, 512
    je .done
    or eax, 0b11 | (1 << 9) ; present + writable
    mov [ebx], eax
    add eax, 4096
    add ebx, 8
    inc ecx
    jmp .next_p1_entry
.done:
    pop ecx
    ret


enable_paging:
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, p4_table - KERNEL_MAPPING_BASE
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8 ; enable long mode
    or eax, 1 << 11 ; enable no execute
    or eax, 1 << 0 ; enable syscall/sysret instructions
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

check_sse:
    mov eax, 0x1
    cpuid
    test edx, 1<<25
    jz .no_sse
    ret
.no_sse:
    mov al, '4'
    jmp error

enable_sse:
    mov eax, cr0
    and ax, 0xFFFB
    or ax, 0x2
    mov cr0, eax
    mov eax, cr4
    or ax, 3 << 9
    mov cr4, eax
    ret

bits 64
prestart64:
    mov rax, start64
    jmp rax

section .text
bits 64
start64:
    mov rsp, stack_top
    mov rax, gdt64.pointer
    lgdt [rax]

    ; update selectors
    mov ax, gdt64.kernel_data
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp start64_2

start64_2:
    mov rax, p4_table
    mov qword [rax], 0
    invlpg [rax]
    mov edi, esi
    call kmain
    cli
.halt:
    hlt
    jmp .halt

section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096
p1_tables:
    resb 4096 * 512
stack_bottom: ; code segment
    resb 3 * 4096
stack_top:
bootinfo: resb 4

section .rodata
align 8
gdt64:
    dq 0                                                                                                ; zero entry
.kernel_code: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53)  ; kernel code segment
.kernel_data: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41)                                          ; kernel data segment
.user_code: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53)  | (1<<46) | (1<<45) ; user code segment
.user_data: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) | (1<<46) | (1<<45) ; user data segment
.end:
.pointer:
    dw gdt64.end - gdt64 - 1
    dq gdt64
.pointer_low:
    dw gdt64.end - gdt64 - 1
    dq gdt64 - KERNEL_MAPPING_BASE