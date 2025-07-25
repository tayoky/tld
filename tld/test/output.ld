ENTRY(_start)
OUTPUT_FORMAT(binary)

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
