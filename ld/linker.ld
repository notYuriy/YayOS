ENTRY(start)

SECTIONS {
    . = 1M;

    .header :{
        KEEP(*(.header))
    }

    .inittext : {
         *(.inittext)
    }
    KERNEL_MAPPING_BASE = 0xffff800000000000;
    . += KERNEL_MAPPING_BASE;

    .text : AT(ADDR(.text) - KERNEL_MAPPING_BASE) {
        *(.text)
    }

    . = ALIGN(4096); 

    .bss : AT(ADDR(.bss) - KERNEL_MAPPING_BASE) {
        *(.bss)
    }

    . = ALIGN(4096); 

    .rodata : AT(ADDR(.rodata) - KERNEL_MAPPING_BASE) {
        *(.rodata)
    }

    . = ALIGN(4096); 
        
    .data : AT(ADDR(.data) - KERNEL_MAPPING_BASE) {
        *(.data)
    }
}