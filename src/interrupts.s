bits 64
section .text

global intLoadIDT

intLoadIDT:
    lidt [rdi]
    ret