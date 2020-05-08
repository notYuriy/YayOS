bits 64

global syscallHandler
section .text

syscallHandler:
    sti
    int3
