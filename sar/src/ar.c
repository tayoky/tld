#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#ifdef __unix__
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <libgen.h>
#include "ar.h"

//no cross plateform way to truncate :(
#ifdef __unix__
#define trunc(file,size) ftruncate(fileno(file),size)
#elif defined(_WIN32)
#define trunc(file,size) _chsize(fileno(file),size)
#else
#define trunc(file,size)
#endif

#define ALIGN(x) (((x)+1)/2*2)

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

long int str2i(char *str,size_t len,int base){
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

void copy_file(FILE *ar,size_t len,FILE *f){
	char buf[4096];
	size_t r;
	size_t total = 0;
	while((r = fread(buf,1,(len - total) > sizeof(buf) ? sizeof(buf) : len - total,ar))){
		fwrite(buf,r,1,f);
		total += r;
	}
	if(total != len){
		error("file too small");
		exit(EXIT_FAILURE);
	}
	fseek(ar,-len,SEEK_CUR);
}



void append_file(FILE *ar,const char *path){
	struct ar_hdr header;
	memset(&header,' ',sizeof(header));
	FILE *f = fopen(path,"r");
	if(!f){
		perror(path);
		exit(EXIT_FAILURE);
	}
	char *name = strdup(path);
	strcpy(header.ar_name,basename(name));
	free(name);
	header.ar_name[strlen(header.ar_name)] = '/';
	fseek(f,0,SEEK_END);
	size_t size = ftell(f);
	snprintf(header.ar_size,10,"%-10zu",size);
	memcpy(header.ar_fmag,ARFMAG,2);
	time_t date = time(NULL);
	snprintf(header.ar_date,12,"%-12ld",(long)date);
#ifdef __unix__
	struct stat st;
	if(stat(path,&st) < 0){
		perror(path);
		exit(EXIT_FAILURE);
	}
	snprintf(header.ar_uid,6,"%-6ld",(long)st.st_uid);
	snprintf(header.ar_gid,6,"%-6ld",(long)st.st_gid);
	snprintf(header.ar_mode,8,"%-8lo",(long)st.st_mode);
#endif
	fwrite(&header,sizeof(header),1,ar);
	rewind(f);
	copy_file(f,size,ar);
	//align on even byte
	if(size % 2){
		fputc('\n',ar);
	}
	fclose(f);
}

int contain_file(int argc,char **argv,const char *name){
	if(argc < 4)return 1;
	for(int i=3; i<argc; i++){
		if(!strcmp(argv[i],name))return 1;
	}
	return 0;
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
		if(file){
			char magic[SARMAG];
			if(!fread(magic,SARMAG,1,file)){
				perror("read");
				return EXIT_FAILURE;
			}
			if(memcmp(magic,ARMAG,SARMAG)){
				error("invalid magic number");
				return EXIT_FAILURE;
			}
		} else {
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

	//d with no file do nothing
	if(strchr(argv[1],'d') && argc < 4){
		return 0;
	}
	if((strchr(argv[1],'r') || strchr(argv[1],'q')) && argc < 4){
		error("no file specified");
		return EXIT_FAILURE;
	}

	//for operations on all files
	fseek(file,SARMAG,SEEK_SET);
	for(;;){
		if(ftell(file) == size)break;
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
		size_t len = str2i(header.ar_size,10,10);
		unsigned int mode = str2i(header.ar_mode,8,8);
		time_t modify = str2i(header.ar_date,12,10);
		struct tm *date = localtime(&modify);
		if(!contain_file(argc,argv,name) || !strcmp(name,"")){
			goto skip;
		}
		if(strchr(argv[1],'d') || strchr(argv[1],'r')){
			//delete
			char buf[4096];
			fseek(file,-sizeof(header),SEEK_CUR);
			off_t cont = ftell(file);
			//now we need to shify cur + len + hdr_sz -> cur
			off_t dest = ftell(file);
			off_t src = dest + sizeof(header) + ALIGN(len);
			for(;;){
				fseek(file,src,SEEK_SET);
				size_t r = fread(buf,1,sizeof(buf),file);
				if(!r)break;
				src = ftell(file);
				fseek(file,dest,SEEK_SET);
				fwrite(buf,r,1,file);
				dest = ftell(file);
			}
			size -= ALIGN(len) + sizeof(header);
			trunc(file,size);
			fseek(file,cont,SEEK_SET);
			goto skip_noseek;

		} else if(strchr(argv[1],'x')){
			//extract
			FILE *f = fopen(name,"w");
			if(!f){
				perror(name);
			} else {
				copy_file(file,len,f);
				fclose(f);
			}
		} else if(strchr(argv[1],'t')){
			//print table
			if(strchr(argv[1],'v')){
#define MODEC(m,c) putchar(mode & m ? c : '-')
				MODEC(0400,'r');
				MODEC(0200,'w');
				MODEC(0100,'x');
				MODEC(0040,'r');
				MODEC(0020,'w');
				MODEC(0010,'x');
				MODEC(0004,'r');
				MODEC(0002,'w');
				MODEC(0001,'x');

				char buf[256];
				strftime(buf,sizeof(buf),"%h %d %h %H:%M %Y",date);
				printf(" %lu/%lu %6zu %s ",str2i(header.ar_uid,6,10),str2i(header.ar_gid,6,10),len,buf);
			}
			puts(name);
		} else if(strchr(argv[1],'p')){
			//print content
			if(strchr(argv[1],'v')){
				printf("%s:\n",name);
			}
			copy_file(file,len,stdout);
		}
skip:
		fseek(file,(len + 1)/2 *2,SEEK_CUR);
skip_noseek:
		free(name);
	}

	if(strchr(argv[1],'q') || strchr(argv[1],'r')){
		//append files
		for(int i=3; i<argc; i++){
			append_file(file,argv[i]);
		}
	}

	return 0;
}
