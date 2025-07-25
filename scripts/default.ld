ENTRY(_start)

PHDRS {
	text    PT_LOAD    FLAGS(0x5);
	data    PT_LOAD    FLAGS(0x6);
	interp  PT_INTERP ;
	dynamic PT_DYNAMIC ;
	note    PT_NOTE ;
}

SECTIONS { 
	. = 0x10000; 
	.text ALIGN(4K) : { 
		*(.text .text.*) 
	} 
	.data ALIGN(4K) : { 
		*(.data .data.*)
		*(.rodata .rodata.*)
	} 
	.bss ALIGN(4K) : { 
		*(.bss)
	} 
}
