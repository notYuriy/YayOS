# YayOS

MIT License applies to every single file in the source code except src/boot/boot.s
This file is an updated version of boot.s from the first edition of blog_os ("https://github.com/phil-opp/blog_os") and it is licensed under src/boot/LICENSE-MIT

For now, it runs a program in /bin/init file. It is a simple assembly program, here is its source.

    bits 64

    section .bss
    data: resb 8

    section .text
        global _start
    _start:
        xor rax, rax ; call fork for the first time
        int 57h
        xor rax, rax ; call fork for the second time
        int 57h
        xor rax, rax ; call fork for the third time
        int 57h
        xor rax, rax ; call fork for the fourth time
        int 57h
        mov qword [data], rax ; test CoW by writing something
        ; to [data]
        mov rax, 29 ; print "Hello world!" on the screen.
        ; Yes, we have a system call for that now
        int 57h
    .exit:
        mov rax, 1 ; terminate process
        int 57h


   