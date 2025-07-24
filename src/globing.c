#include <string.h>
#include <libgen.h>
#include "tld.h"

//i found this very cool aglo online :D
int glob_match(const char *pattern,const char *str){
	size_t strx = 0;
	size_t patx = 0;
	size_t strx_start = 0;
	size_t patx_start = 0;

	while(pattern[patx] || str[strx]){
		switch(pattern[patx]){
		case '\0':
			break;
		case '?':
			if(strx < strlen(str)){
				strx++;
				patx++;
				continue;
			}
			break;
		case '*':
			//try to find the seach
			//if not found retry onz char later
			patx_start = patx;
			strx_start = strx + 1;
			patx++;
			continue;
		default:
			if(strx < strlen(str) && str[strx] == pattern[patx]){
				strx++;
				patx++;
				continue;
			}
			break;
		}
		if(0 < strx_start && strx_start <= strlen(str)){
			patx = patx_start;
			strx = strx_start;
			continue;
		}
		return 0;
	}
	return 1;
}

int glob_path_match(const char *patern,const char *str){
	if(strchr(patern,'/')){
		return glob_match(patern,str);
	}

	char d[strlen(str)+1];
	strcpy(d,str);
	return glob_match(patern,basename(d));
}
