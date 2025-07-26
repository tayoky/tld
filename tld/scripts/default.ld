ENTRY(_start)

PHDRS {
	text    PT_LOAD    FLAGS(0x5);
	data    PT_LOAD    FLAGS(0x6);
	interp  PT_INTERP ;
	dynamic PT_DYNAMIC ;
	note    PT_NOTE ;
}

SECTIONS { 
	. = __tld_cmd_Ttext ; 
	.text ALIGN(4K) : { 
		*(.text .text.*) 
		*(.init .init.*)
		*(.fini .fini.*)
	} 
	.init_array ALIGN(4K) : {
		__init_array_start = . ;
		*(.init_array .init_array.*)
		__init_array_end = . ;
	}
	.fini_array ALIGN(4K) : {
		__fini_array_start = . ;
		*(.fini_array .fini_array.*)
		__fini_array_end = . ;
	}
	.data ALIGN(4K) : { 
		*(.data .data.*)
		*(.rodata .rodata.*)
	} 
	.bss ALIGN(4K) : { 
		*(.bss)
	} 
}
