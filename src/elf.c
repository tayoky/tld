#include "elf.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "tld.h"

#define BITS 64
#define _TYPEDEF(bits,suffix) typedef Elf##bits##_##suffix Elf_##suffix;
#define TYPEDEF(bits) _TYPEDEF(bits,Ehdr)\
	_TYPEDEF(bits,Shdr)

TYPEDEF(64)

int elf64_load(tld_file *file){
	rewind(file->file);
	Elf_Ehdr header;
	if(!fread(&header,sizeof(header),1,file->file))return -1;
	errno = EINVAL;
	if(header.e_ident[EI_VERSION] != EV_CURRENT || header.e_ident[EI_CLASS] != ELFCLASS64 || header.e_type != ET_REL){
		error("file is not a 64 bit reloctable");
		return -1;
	}

	Elf_Shdr shstrtab_header;
	fseek(file->file,header.e_shoff + header.e_shentsize * header.e_shstrndx,SEEK_SET);
	if(!fread(&shstrtab_header,sizeof(shstrtab_header),1,file->file))return -1;

	char *shstrtab = malloc(shstrtab_header.sh_size);
	if(fseek(file->file,shstrtab_header.sh_offset,SEEK_SET) < 0)return -1;
	if(!fread(shstrtab,shstrtab_header.sh_size,1,file->file))return -1;

	file->sections_count = header.e_shnum - 1;
	file->sections = calloc(file->sections_count,sizeof(tld_section));

	for(size_t i=0; i<file->sections_count; i++){
		if(fseek(file->file,header.e_shoff + header.e_shentsize * (i + 1),SEEK_SET) < 0)return -1;
		Elf_Shdr sheader;
		if(!fread(&sheader,sizeof(sheader),1,file->file)){
			free(shstrtab);
			return -1;
		}
		printf("find section %s of size %lu\n",&shstrtab[sheader.sh_name],sheader.sh_size);
		file->sections[i].size = sheader.sh_size;
		file->sections[i].name = strdup(&shstrtab[sheader.sh_name]);
		file->sections[i].data = malloc(sheader.sh_size);
		fseek(file->file,sheader.sh_offset,SEEK_SET);
		fread(file->sections[i].data,sheader.sh_size,1,file->file);

	}
	return 0;
}

int elf_load(tld_file *file){
	return elf64_load(file);
}
