#include <stdlib.h>
#include <stdio.h>
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
		}
		rewind(fd);
	}
	return file;
error:
	fclose(fd);
	free(file);
	return NULL;
}

void tld_close_file(tld_file *file){
	//TODO : write the sections
	fclose(file->file);
	free(file);
}
