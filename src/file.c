#include <stdlib.h>
#include <stdio.h>
#include "tld.h"

//top domain file management and abstraction

tld_file *tld_open_file(const char *path,const char *mode){
	FILE *fd = fopen(path,mode);
	if(!fd)return NULL;
	tld_file *file = malloc(sizeof(tld_file));
	memset(file,0,sizeof(tld_file));
	file->file = fd;
	if(mode[0] == 'r'){
		//TODO : auto detect format
	}
	return file;
}

void tld_close_file(tld_file *file){
	//TODO : write the sections
	fclose(file->file);
	free(file);
}
