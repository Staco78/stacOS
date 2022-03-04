ENTRY(_start)
SECTIONS 
{
    . = 0x1000;

    .apBoot : {
        *(.apBoot)
    }

    KERNEL_VMA = 0xFFFFFFFF80000000;
    . = 1M;
    
    .multiboot : {
        *(.multiboot)
    }

    .boot.text : {
        *(.boot.text)
    }

    .boot.data : {
        *(.bot.data)
        . = ALIGN(4096);
    }

    . += KERNEL_VMA;

    .text : AT(ADDR(.text) - KERNEL_VMA) {
        *(.text)
        . = ALIGN(4096);
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) {
        *(.rodata)
        . = ALIGN(4096);
    }

    .data : AT(ADDR(.data) - KERNEL_VMA) {
        *(.data)
        . = ALIGN(4096);
    }

    .eh_frame : AT(ADDR(.eh_frame) - KERNEL_VMA) {
        *(.eh_frame)
        . = ALIGN(4096);
    }

    .bss : AT(ADDR(.bss) - KERNEL_VMA) {
        *(.bss)

        *(COMMON)
        . = ALIGN(4096);
    }

    _end = .;

    /DISCARD/ : {
        *(.comment)
    }
    
}