ENTRY(start)

SECTIONS {
    . = 1M;

    _kernel_start =.;
    _kern_virtual_offset = 0xffffffff80000000;
    .multiboot_header :
    {
        /* Be sure that multiboot header is at the beginning */
        *(.multiboot_header)
    }

    .multiboot.text :
    {
        *(.multiboot.text)
    }

    . += _kern_virtual_offset;
	/* Add a symbol that indicates the start address of the kernel. */
	.text ALIGN (4K) : AT (ADDR (.text) - _kern_virtual_offset)
	{
		*(.text)
		*(.text.*)
	}
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - _kern_virtual_offset)
	{
		*(.rodata)
		*(.rodata.*)
	}
	.data ALIGN (4K) : AT (ADDR (.data) - _kern_virtual_offset)
	{
		  start_ctors = .;
          KEEP(*( .init_array ));
          KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
          end_ctors = .;

          *(.data)
         *(.data.*)
	}
	.bss ALIGN (4K) : AT (ADDR (.bss) - _kern_virtual_offset)
	{
		*(.bss)
	}

    _kernel_end = .;
    _kernel_size = _kernel_end - _kernel_start - _kern_virtual_offset;
    _kernel_physical_end = . - _kern_virtual_offset;
}