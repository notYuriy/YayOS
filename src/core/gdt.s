bits 64

section .text
global loadGDT
loadGDT:
    lgdt [rdi]
    ret