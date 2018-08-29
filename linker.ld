OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
/* STARTUP("boot/stage1.o")*/
SECTIONS
{
    . = 0x008000;

    .text :
    {
        *(.text)
		*(.lowtext)
        *(.gnu.linkonce.t*)
    }

    .rodata ALIGN(0x1000) :
    {
        start_ctors = .;
        *(SORT(.ctors*))  /* Note the "SORT" */
        *(SORT(.init_array*))
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        *(SORT(.fini_array*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
    }

    .data ALIGN(0x1000) :
    {
        *(.data)
		*(.lowdata)
        *(.gnu.linkonce.d*)
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
