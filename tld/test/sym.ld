SECTIONS {
	. = 0x400000;
	test = . ;
	__text_start = . ;
	.text ALIGN(4K) : {
		*(.text .text.*)
	}
	__data_start = . ;
	.data ALIGN(4K) : {
		*(.data .data.*)
		*(.rodata .rodata.*)
	}
	__bss_start = . ;
	.bss ALIGN(4K) : {
		*(.bss .bss.*)
	}
	_bss_end = . ;
}
