/* The execution should start at _start */
ENTRY(_start)

OUTPUT_FORMAT(binary)

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
    }
    .data ALIGN(4096) : {
        *(.data)
    }
    .rodata ALIGN(4096) : {
        *(.rodata)
    }
    .bss ALIGN(4096) : {
        *(.bss)
    }
}