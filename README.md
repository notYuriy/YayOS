# YayOS

MIT License applies to every single file in the source code except src/boot/boot.s
This file is an updated version of boot.s from the first edition of blog_os ("https://github.com/phil-opp/blog_os") and it is licensed under src/boot/LICENSE-MIT

YayOS is a monolitic hobby operating system powered by YYSloth kerner. For now, it is only capable of runnning one executable from ramdisk (stored in the file /bin/init).

The API provided by the system is extremely simple. Programs can print to serial port, duplicate (similarly to fork), terminate, yield, and get system info (similarly to uname).

The "/bin/init" program was assembled from the following code

```nasm
bits 64

section .bss
align 4096
stackBottom:
resb 4096
stackTop:
systemInfoBuffer: resb 455

kernelNameOffset: equ 0
kernelReleaseOffset: equ 65
kernelVersionOffset: equ 130
machineOffset: equ 195
processorOffset: equ 260
hardwarePlatformOffset: equ 325
operatingSystemOffset: equ 390
systemInfoStringSize: equ 64

section .data
kernelNameString: db "kernelName: "
kernelNameLen: equ $ - kernelNameString

kernelReleaseString: db "kernelRelease: "
kernelReleaseLen: equ $ - kernelReleaseString

kernelVersionString: db "kernelVersion: "
kernelVersionLen: equ $ - kernelVersionString

machineString: db "machine: "
machineLen: equ $ - machineString

processorString: db "processor: "
processorLen: equ $ - processorString

hardwarePlatformString: db "hardwarePlatform: "
hardwarePlatformLen: equ $ - hardwarePlatformString

operatingSystemString: db "operatingSystem: "
operatingSystemLen: equ $ - operatingSystemString

termString: db 13, 10
termLen: equ $ - termString

section .text

; YY_Console_Write syscall takes two arguments:
; rdi - start of the string
; rsi - size of the string
YY_ConsoleWrite: equ 2

; YY_GetSystemInfo takes one argument
; rdi - buffer to store system info into
YY_GetSystemInfo: equ 3

; YY_ExitProcess doesn't take any arguments
; Return codes are yet to implement
YY_ExitProcess: equ 0

global _start

; r9 - hint message
; r10 - hint length
; rdi - offset in systemInfo

printParam:
    push rdi
    
    mov rdi, r9
    mov rsi, r10
    mov rax, YY_ConsoleWrite
    int 57h
    
    pop rdi
    
    add rdi, systemInfoBuffer
    mov rsi, systemInfoStringSize
    mov rax, YY_ConsoleWrite
    int 57h
    
    mov rdi, termString
    mov rsi, termLen
    mov rax, YY_ConsoleWrite
    int 57h
    ret
    
%macro PRINT_PARAM 1
    mov r9, %1String
    mov r10, %1Len
    mov rdi, %1Offset
    call printParam
%endmacro

_start:
    mov rsp, stackTop
    
    mov rdi, systemInfoBuffer
    mov rax, YY_GetSystemInfo
    int 57h
    
    PRINT_PARAM kernelName
    PRINT_PARAM kernelRelease
    PRINT_PARAM kernelVersion
    PRINT_PARAM machine
    PRINT_PARAM processor
    PRINT_PARAM hardwarePlatform
    PRINT_PARAM operatingSystem
    
    mov rax, YY_ExitProcess
    int 57h
```