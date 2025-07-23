#include <string.h>
#include <libgen.h>
#include "tld.h"

//TODO : replace with some actual globing
int glob_match(const char *patern,const char *str){
	if(!strcmp(patern,"*"))return 1;
	return !strcmp(patern,str);
}

int glob_path_match(const char *patern,const char *str){
	if(strchr(str,'/')){
		return glob_match(patern,str);
	}

	char d[strlen(str+1)];
	strcpy(d,str);
	return glob_match(patern,basename(d));
}
