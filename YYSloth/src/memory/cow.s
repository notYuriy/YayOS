bits 64
section .text

%include "proc/state.inc"

global pageFaultHandler
extern pageFaultHandlerWithFrame

pageFaultHandler:
    PUSHREGS_WITH_CODE
    call pageFaultHandlerWithFrame
    POPREGS
    iretq