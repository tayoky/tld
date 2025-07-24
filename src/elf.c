#include "elf.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "tld.h"

//generic driver for elf files

	
int elf_load(tld_file *file){
	rewind(file->file);
	Elf64_Ehdr header;
	if(!fread(&header,sizeof(header),1,file->file))return -1;
	switch(header.e_ident[EI_CLASS]){
	case ELFCLASS64:
		return elf64_load(file);
	case ELFCLASS32:
		return elf32_load(file);
	default:
		error("invalid elf class");
		return -1;
	}
}
