#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tdbg.h"
#include "libttc.h"

const char *progname = "tdbg";

int main(int argc,char **argv){
	tdbg_state context;
	memset(&context,0,sizeof(context));
	if(argc >= 2 && argv[1][0] != '-'){
		context.exe_args = malloc(sizeof(char*[2]));
		context.exe_args[0] = strdup(argv[1]);
		context.exe_args[1] = NULL;
	}
	for(;;){
		printf("$ ");
		fflush(stdout);
		char buf[4096];
		fgets(buf,sizeof(buf),stdin);
		cmd(&context,buf);
	}
	return 0;
}
