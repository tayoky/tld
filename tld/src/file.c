#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <libgen.h>
#include "elf.h"
#include "tld.h"

//top domain file management and abstraction

tld_file *tld_open_file(const char *path,const char *mode){
	FILE *fd = fopen(path,mode);
	if(!fd)return NULL;
	tld_file *file = malloc(sizeof(tld_file));
	memset(file,0,sizeof(tld_file));
	file->file = fd;
	if(mode[0] == 'r'){
		//auto detect format
		char magic[4];
		if(!fread(magic,sizeof(magic),1,fd))goto error;
		if(!memcmp(magic,ELFMAG,4)){
			//elf file
			if(elf_load(file) < 0){
				free(file->sections);
				fclose(fd);
				free(file);
				return NULL;
			}
		} else {
			//binary file
			file->sections = malloc(sizeof(tld_section));
			fseek(fd,0,SEEK_SET);
			file->sections[0].size = ftell(fd);
			rewind(fd);
			file->sections[0].data = malloc(file->sections[0].size);
			fread(file->sections[0].data,file->sections[0].size,0,fd);
			file->sections[0].name = strdup(".bin");
		}
		rewind(fd);
	}
	file->name = strdup(path);
	return file;
error:
	fclose(fd);
	free(file);
	return NULL;
}

void tld_close_file(tld_file *file){
	for(size_t i=0; i<file->sections_count; i++){
		free(file->sections[i].name);
		free(file->sections[i].data);
		free(file->sections[i].relocs);
	}
	for(size_t i=0; i<file->symbols_count; i++){
		free(file->symbols[i].name);
	}
	free(file->name);
	free(file->sections);
	free(file->symbols);
	fclose(file->file);
	free(file);
}

int tld_save_file(tld_file *file,int format,int arch){
	if(!file->phdrs){
		//no phdrs ?
		//generate default one
		file->phdrs_count = file->sections_count;
		file->phdrs = calloc(file->phdrs_count,sizeof(tld_phdr));
		for(size_t i=0; i<file->phdrs_count; i++){
			file->phdrs[i].name = strdup(file->sections[i].name);
			file->phdrs[i].flags = 0x7;
			file->phdrs[i].type = 1;
			file->phdrs[i].sections_count = 0;
			file->phdrs[i].first_section = i;
		}
	}

	switch(format){
	case FORMAT_ELF64:
		return elf64_save(file,arch);
	case FORMAT_ELF32:
		return elf32_save(file,arch);
	case FORMAT_BINARY:
		return bin_save(file,arch);
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}

tld_symbol *tld_get_sym(tld_file *file,const char *name){
	size_t i = 0;
	for(; i<file->symbols_count; i++){
		if(!strcmp(file->symbols[i].name,name) && !(file->symbols[i].flags & TLD_SYM_LOCAL))break;
	}
	if(i == file->symbols_count){
		error("undefined reference to %s",name);
		return NULL;
	}
	return &file->symbols[i];
}

int str2format(const char *str){
	if(!strncmp(str,"bin",3)){
		return FORMAT_BINARY;
	}
	if(!strncmp(str,"elf",3)){
		str += 3;
		if(*str == '-')str++;
		if(!strcmp(str,"32")){
			return FORMAT_ELF32;
		} else {
			return FORMAT_ELF64;
		}
	}
	errno = EINVAL;
	return -1;
}
