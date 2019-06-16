OUTPUT_FORMAT("elf32-i386")
ENTRY(_multibootEntry)

SECTIONS
{
    . = 0x0100000;

	.header :
	{
		*(.header)
		_end_header = .;
		. = ALIGN(0x1000);
		start_load = .;
	}
	_begin_trampoline = .;
	.lowmem 0x8000:
	AT(_begin_trampoline)
	{
		_low_mapping = .;
		*(.lowtext)
		*(.lowdata)
		. = ALIGN(0x1000);
	}
	. = _begin_trampoline  + SIZEOF(.lowmem);
	_end_trampoline = .;

    .text :
	AT(ADDR(.text))
	{
        *(.text);
        *(.gnu.linkonce.t*);
    }

    .rodata ALIGN(0x1000) :
    {
        start_ctors = .;
        *(SORT(.ctors*))  /* Note the "SORT" */
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)

    }

    .data ALIGN(0x1000) :
    {
        *(.data)
        *(.gnu.linkonce.d*)
		end_load = .;
    }

    .bss :
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        *(.gnu.linkonce.b*)
        ebss = .;
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame) /* c++ exception stuff  */
    }
}
