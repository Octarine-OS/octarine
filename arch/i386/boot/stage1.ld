OUTPUT_FORMAT("binary")
/*ENTRY(_start)*/
/* STARTUP("boot/stage1.o")*/
SECTIONS
{
    . = 0x7C00;

    .text :
    {
        *(.text)
        *(.gnu.linkonce.t*)
    }

/*
    .rodata ALIGN(0x1000) :
    {
        start_ctors = .;
        *(SORT(.ctors*))  
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
    }

    .bss :
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        *(.gnu.linkonce.b*)
        ebss = .;
    }
*/
    /DISCARD/ :
    {
        *(.bss) /* TODO, am I sure I want to throw away BSS? */
        *(.comment)
        *(.eh_frame) /* c++ exception stuff  */
    }
}

