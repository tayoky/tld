#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "ar.h"

// cute custom perror
#undef perror
#define perror(str) error("%s : %s",str,strerror(errno))
#define warning error

void error(const char *fmt,...){
	va_list args;
	va_start(args,fmt);
	fputs("sar : ",stderr);
	vfprintf(stderr,fmt,args);
	fputc('\n',stderr);
	va_end(args);
}

char *get_name(struct ar_hdr *header){
	if(memchr(header->ar_name,'/',sizeof(header->ar_name))){
		*(char *)memchr(header->ar_name,'/',sizeof(header->ar_name)) = '\0';
	} else if(memchr(header->ar_name,' ',sizeof(header->ar_name))){
		*(char *)memchr(header->ar_name,' ',sizeof(header->ar_name)) = '\0';
	} else {
		error("unsupported name type");
		exit(EXIT_FAILURE);
	}
	return strdup(header->ar_name);
}

size_t str2i(char *str,size_t len,int base){
	while((*str == ' ' || *str == '0') && len > 0){
		str++;
		len--;
	}
	size_t i=0;
	while(len > 0 && *str != ' '){
		i *= base;
		i += *str - '0';
		str++;
		len--;
	}
	return i;
}

int main(int argc,char **argv){
	if(argc < 3){
		error("not enought arguments");
		return EXIT_FAILURE;
	}
	FILE *file;
	if(strchr(argv[1],'c')){
		file = fopen(argv[2],"w+");
		if(file)fputs(ARMAG,file);
	} else {
		file = fopen(argv[2],"r+");
		if(!file){
			warning("creating archive");
			file = fopen(argv[2],"w+");
			if(file)fputs(ARMAG,file);
		}
	}
	if(!file){
		perror(argv[1]);
		return EXIT_FAILURE;
	}
	fseek(file,0,SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	if(strchr(argv[1],'t')){
		//print
		fseek(file,SARMAG,SEEK_SET);
		for(;;){
			struct ar_hdr header;
			if(!fread(&header,sizeof(struct ar_hdr),1,file)){
				perror("read");
				return EXIT_FAILURE;
			}
			if(memcmp(header.ar_fmag,ARFMAG,2)){
				error("invalid ar header");
				return 1;
			}
			char *name = get_name(&header);
			if(strcmp(name,""))puts(name);
			free(name);
			size_t len = str2i(header.ar_size,10,10);
			fseek(file,(len + 1)/2 *2,SEEK_CUR);
			if(ftell(file) == size)break;
		}

	}
	return 0;
}
