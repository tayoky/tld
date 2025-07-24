#include <string.h>
#include <stdio.h>
#include "tld.h"

static void i386_reloc(tld_section *section,tld_reloc *reloc,tld_symbol *sym){
	uint32_t A;
	memcpy(&A,&section->data[reloc->offset],sizeof(uint32_t));
	uint32_t result;
	uint32_t S = sym->offset;
	uint32_t P = section->address + reloc->offset;
	switch(reloc->type){
	case R_386_32:
		result = S + A;
		break;
	case R_386_PC32:
		result = S + A - P;
		break;
	}
	memcpy(&section->data[reloc->offset],&result,sizeof(uint32_t));
}

static void apply_reloc(tld_file *file,tld_section *section,tld_reloc *reloc,int arch){
	tld_symbol *sym = reloc->symbol;
	if(sym->flags & TLD_SYM_UNDEF){
		//search a global or weak symbol with same name in global table
		size_t i = 0;
		for(; i<file->symbols_count; i++){
			if(!strcmp(file->symbols[i].name,sym->name) && !(file->symbols[i].flags & TLD_SYM_LOCAL))break;
		}
		if(i == file->symbols_count){
			error("undefined reference to %s",sym->name);
			return;
		}
		sym = &file->symbols[i];
	} else {
		sym = &file->symbols[sym->linked];
	}
	if(!sym){
		error("symbol %s is not present in executable",reloc->symbol->name);
		return;
	}

	switch(arch){
	case ARCH_I386:
		return i386_reloc(section,reloc,sym);
	}
}

//try to apply as much relocation as we can
void tld_apply_relocations(tld_file *file,int arch){
	for(size_t i=0; i<file->sections_count; i++){
		for(size_t j=0; j<file->sections[i].relocs_count; j++){
			apply_reloc(file,&file->sections[i],&file->sections[i].relocs[j],arch);
		}
	}
}
