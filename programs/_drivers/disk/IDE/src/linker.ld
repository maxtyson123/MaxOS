/* The execution should start at _start */
ENTRY(_start)

OUTPUT_FORMAT(elf64-x86-64)

/*
 * Here it is determined in which order which sections in the binary
 * are to be written
 */
SECTIONS
{
    /* The program is loaded at 2 MB, Will be changed later for loading from filw */
    . = 0x200000;

    .text : {
        *(.text)
        *(.text.*)
    }
    .data ALIGN(4k) : {
        *(.data)
        *(.data.*)
    }
    .rodata ALIGN(4k) : {
        *(.rodata)
        *(.rodata.*)
    }
    .bss ALIGN(4k) : {
        *(.bss)
        *(.bss.*)
    }
}