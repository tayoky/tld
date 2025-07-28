#include <string.h>
#include <stdio.h>
#include "tld.h"

#define SIZE_8  1
#define SIZE_16 2
#define SIZE_32 3
#define SIZE_64 4

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

static void x86_64_reloc(tld_section *section,tld_reloc *reloc,tld_symbol *sym){
	uint64_t A = reloc->addend;
	uint64_t result;
	uint64_t S = sym->offset;
	uint64_t P = section->address + reloc->offset;
	int size;
	switch(reloc->type){
	case R_X86_64_NONE:
		return;
	case R_X86_64_64:
		size = SIZE_64;
		result = S + A;
		break;
	case R_X86_64_PC64:
		size = SIZE_64;
		result = S + A - P;
		break;
	case R_X86_64_32:
	case R_X86_64_32S:
		size = SIZE_32;
		result = S + A;
		break;
	case R_X86_64_PC32:
		size = SIZE_32;
		result = S + A - P;
		break;
	case R_X86_64_16:
		size = SIZE_16;
		result = S + A;
		break;
	case R_X86_64_PC16:
		size = SIZE_16;
		result = S + A - P;
		break;
	case R_X86_64_8:
		size = SIZE_8;
		result = S + A;
		break;
	case R_X86_64_PC8:
		size = SIZE_8;
		result = S + A - P;
		break;
	case R_X86_64_GLOB_DAT:
	case R_X86_64_JUMP_SLOT:
		size = SIZE_64;
		result = S;
		break;
	}
	switch(size){
	case SIZE_64:
		memcpy(&section->data[reloc->offset],&result,sizeof(uint64_t));
		break;
	case SIZE_32:;
		uint32_t result32 = (uint32_t)result;
		memcpy(&section->data[reloc->offset],&result32,sizeof(uint32_t));
	case SIZE_16:;
		uint16_t result16 = (uint16_t)result;
		memcpy(&section->data[reloc->offset],&result16,sizeof(uint16_t));
	case SIZE_8:;
		uint8_t result8 = (uint8_t)result;
		memcpy(&section->data[reloc->offset],&result8,sizeof(uint8_t));
		break;
	}
#ifdef DEBUG
	printf("reloc of value %lx at %lx\n",result,P);
#endif
}

static void apply_reloc(tld_file *file,tld_section *section,tld_reloc *reloc,int arch){
	tld_symbol *sym = reloc->symbol;
	if(sym->flags & TLD_SYM_UNDEF){
		//search a global or weak symbol with same name in global table
		sym = tld_get_sym(file,sym->name);
		if(!sym)return;
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
	case ARCH_X86_64:
		return x86_64_reloc(section,reloc,sym);
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
