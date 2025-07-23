#include "elf.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "tld.h"

#define BITS 64
#define _TYPEDEF(bits,suffix) typedef Elf##bits##_##suffix Elf_##suffix;
#define TYPEDEF(bits) _TYPEDEF(bits,Ehdr)\
	_TYPEDEF(bits,Shdr)\
	_TYPEDEF(bits,Phdr)

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

#define add_string(str) strtab = realloc(strtab,strtab_len + strlen(str) + 1);\
				 strcpy(&strtab[strtab_len],str);\
				 strtab_len += strlen(str) + 1;

int elf64_save(tld_file *file,int arch){
	puts("save");
	rewind(file->file);
	Elf_Ehdr header;
	memset(&header,0,sizeof(header));
	memcpy(header.e_ident,ELFMAG,SELFMAG);
	header.e_ident[EI_VERSION] = EV_CURRENT;
	header.e_ident[EI_CLASS]   = ELFCLASS64;
	header.e_ehsize = sizeof(header);
	header.e_phentsize = sizeof(Elf_Phdr);
	header.e_phnum = file->sections_count + 1;
	header.e_phoff = sizeof(header);
	header.e_shentsize = sizeof(Elf_Shdr);
	header.e_shnum = file->sections_count + 2;
	header.e_shoff = sizeof(header) + sizeof(Elf_Phdr) * (file->sections_count + 1);
	header.e_shstrndx = file->sections_count + 1;
	if(!fwrite(&header,sizeof(header),1,file->file))return -1;


	//init strtab
	char *strtab = strdup("");
	size_t strtab_len = 1;

	//write phdr
	off_t offset = sizeof(header) + (sizeof(Elf_Phdr) + sizeof(Elf_Shdr)) * (file->sections_count + 1 ) + sizeof(Elf_Shdr);
	Elf_Phdr pheader;
	memset(&pheader,0,sizeof(pheader));
	fwrite(&pheader,sizeof(pheader),1,file->file);
	for(size_t i=0; i<file->sections_count; i++){
		memset(&pheader,0,sizeof(pheader));
		pheader.p_type = PT_LOAD;
		pheader.p_memsz = file->sections[i].size;
		pheader.p_filesz = pheader.p_memsz;
		pheader.p_offset = offset;
		offset += pheader.p_filesz;
		fwrite(&pheader,sizeof(pheader),1,file->file);
	}

	//write shdr
	offset = sizeof(header) + (sizeof(Elf_Phdr) + sizeof(Elf_Shdr)) * (file->sections_count + 1 ) + sizeof(Elf_Shdr);
	Elf_Shdr sheader;
	memset(&sheader,0,sizeof(sheader));
	fwrite(&sheader,sizeof(sheader),1,file->file);
	for(size_t i=0; i<file->sections_count; i++){
		Elf_Shdr sheader;
		memset(&sheader,0,sizeof(sheader));
		sheader.sh_size = file->sections[i].size;
		sheader.sh_name = strtab_len;
		add_string(file->sections[i].name);
		sheader.sh_offset = offset;
		offset += file->sections[i].size;
		fwrite(&sheader,sizeof(sheader),1,file->file);
	}

	//write strtab header
	memset(&sheader,0,sizeof(sheader));
	sheader.sh_type = SHT_STRTAB;
	sheader.sh_offset = offset;
	sheader.sh_name = strtab_len;
	add_string(".strtab");
	sheader.sh_size = strtab_len;
	fwrite(&sheader,sizeof(sheader),1,file->file);

	//write sections
	for(size_t i=0; i<file->sections_count; i++){
		fwrite(file->sections[i].data,file->sections[i].size,1,file->file);
	}

	//write strtab content
	fwrite(strtab,strtab_len,1,file->file);
	return 0;
}

int elf_load(tld_file *file){
	return elf64_load(file);
}
