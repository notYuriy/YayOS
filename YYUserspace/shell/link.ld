ENTRY(_start)

SECTIONS {
    . = 400M;
    .text : {
        *(.text)
    }
    .data : {
        *(.data)
    }
    .bss : {
        *(.bss)
    }
    .rodata : {
        *(.rodata)
    }
}