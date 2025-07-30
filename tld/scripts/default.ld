ENTRY(_start)

PHDRS {
	text    PT_LOAD    FLAGS(0x5);
	data    PT_LOAD    FLAGS(0x6);
	bss     PT_LOAD    FLAGS(0x6);
	note    PT_NOTE ;
}

SECTIONS { 
	. = __tld_cmd_Ttext ; 
	.text ALIGN(4K) : { 
		*(.text .text.*) 
		*(.init .init.*)
		*(.fini .fini.*)
		*(.eh_frame .eh_frame.*)
	} : text
	.init_array : {
		__init_array_start = . ;
		*(.init_array .init_array.*)
		__init_array_end = . ;
	} : text
	.fini_array : {
		__fini_array_start = . ;
		*(.fini_array .fini_array.*)
		__fini_array_end = . ;
	} : text
	.data ALIGN(4K) : { 
		*(.data .data.*)
		*(.rodata .rodata.*)
	} : data
	.bss ALIGN(4K) : { 
		*(.bss)
	} : bss
}
