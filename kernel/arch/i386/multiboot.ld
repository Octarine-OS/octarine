OUTPUT_FORMAT("elf32-i386")
ENTRY(_multibootEntry)

KERNEL_VIRTUAL_BASE = 0xC0000000;
KERNEL_LOAD_ADDRESS = 0x00100000;

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
    lowmem PT_LOAD FLAGS(6);
    bss PT_LOAD FLAGS(6);
}

SECTIONS
{
    . =  KERNEL_LOAD_ADDRESS + KERNEL_VIRTUAL_BASE;

    .text : AT(KERNEL_LOAD_ADDRESS)
	{
        *(.header)
        ASSERT((. > 1), "Malformed multiboot header")
        *(.text);
        *(.gnu.linkonce.t*);
        /*. = ALIGN(0x1000);*/
    } :text
    mboot_phys_entry = ABSOLUTE(_multibootEntry) - KERNEL_VIRTUAL_BASE;
    phys_header_addr = LOADADDR(.text);
    start_load = LOADADDR(.text);
    .rodata ALIGN(0x1000) : AT(ALIGN(LOADADDR(.text) + SIZEOF(.text), 0x1000))
    {
        start_ctors = .;
        *(SORT(.ctors*))  /* Note the "SORT" */
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)

    } :data

    .data /*ALIGN(0x1000)*/ :
    {
        *(.data)
        *(.gnu.linkonce.d*)
        . = ALIGN(0x1000);
    }

    _begin_trampoline = ABSOLUTE(.);
	.lowmem 0x8000: AT(LOADADDR(.data) + SIZEOF(.data))
	{
		_low_mapping = .;
		*(.lowtext)
		*(.lowdata)
		. = ALIGN(0x1000);
	} :lowmem
    _end_trampoline = ABSOLUTE(_begin_trampoline + SIZEOF(.lowmem));

    end_load = LOADADDR(.lowmem) + SIZEOF(.lowmem);

    .bss  _end_trampoline: AT(LOADADDR(.lowmem) + SIZEOF(.lowmem))
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        *(.gnu.linkonce.b*)
    } :bss
    ebss = ABSOLUTE( LOADADDR(.bss) + SIZEOF(.bss));

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame) /* c++ exception stuff  */
    }
}
