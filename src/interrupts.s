bits 64
section .text

global intLoadIdt

intLoadIdt:
    lidt [rdi]
    ret