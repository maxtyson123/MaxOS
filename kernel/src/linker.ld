ENTRY(start)

PHDRS
{
    boot PT_LOAD FLAGS(5);   /* Boot: Read, Execute */
    text PT_LOAD FLAGS(5);   /* Kernel: Read, Execute */
    data PT_LOAD FLAGS(6);   /* Kernel: Read, Write */
}

SECTIONS {

    /* Multiboot must be first */
    .multiboot_header :
    {
        /* Be sure that multiboot header is at the beginning */
        _multiboot_header_start = .;
        *(.multiboot_header)
    } :boot

    .multiboot.text :
    {
        *(.multiboot.text)
    } :boot

    /* Physical location of spinup code */
    .spinup 0x8000 :
    {
        *(.spinup)
    } :boot

    /* Place the kernel at 1MB and link it to -2GB */
    . = 1M;
    _kernel_start =.;
    _kern_virtual_offset = 0xffffffff80000000;
    . += _kern_virtual_offset;

	.text ALIGN (4K) : AT (ADDR (.text) - _kern_virtual_offset)
	{
		*(.text)
		*(.text.*)
	}:text

	.rodata ALIGN (4K) : AT (ADDR (.rodata) - _kern_virtual_offset)
	{
		*(.rodata)
		*(.rodata.*)
	} :data

	.data ALIGN (4K) : AT (ADDR (.data) - _kern_virtual_offset)
	{
		  start_ctors = .;
          KEEP(*( .init_array ));
          KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
          end_ctors = .;

          *(.data)
         *(.data.*)
	} :data

	.bss ALIGN (4K) : AT (ADDR (.bss) - _kern_virtual_offset)
	{
		*(.bss)
	} :data

    /* Kernel Metadata */
    _kernel_end = .;
    _kernel_size = _kernel_end - _kernel_start - _kern_virtual_offset;
    _kernel_physical_end = . - _kern_virtual_offset;
}