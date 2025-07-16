#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tld.h"

typedef struct option {
	const char *option;
	char opt;
	int flag;
	void (*func)(tld_state *,char *);
} option;

#define OPT(s,l,f) {.opt = s,.option = l,.flag = f}
#define OPTA(s,l,f) {.opt = s,.option = l,.func = (void *)f}

void set_output(tld_state *state,char *arg){
	if(state->out){
		error("only one output can be specfied");
		exit(EXIT_FAILURE);
	}
	state->out = fopen(arg,"w");
	if(!state->out){
		perror(arg);
		exit(EXIT_FAILURE);
	}
}
void set_script(tld_state *state,char *arg){
	if(state->out){
		error("only one script can be specfied");
		exit(EXIT_FAILURE);
	}
	state->out = fopen(arg,"r");
	if(!state->out){
		perror(arg);
		exit(EXIT_FAILURE);
	}
}

void set_arch(tld_state *state,const char *arg){

}

option options[] = {
	OPTA('o',"--output",set_output),
	OPTA('T',"--script",set_script),
	OPTA('A',"--architecture",set_arch),
};

void parse_arg(tld_state *state,int argc,char **argv){
	int i=1;
	for(;i<argc;i++){
		if(argv[i][0] != '-')continue;
		if(!strcmp(argv[i],"--")){
			i++;
			break;
		}

		//time to parse arg
		if(argv[i][1] == '-'){
			//long option
			for(size_t j=0; j<arraylen(options); j++){
				if(!options[j].option)continue;
				if(strcmp(options[j].option,argv[i]))continue;
				if(options[j].func){
					if(i+1==argc){
						error("missing operand");
						exit(EXIT_FAILURE);
					}
					options[j].func(state,argv[i+1]);
				}
				break;
			}
		} else {
			//short option
			for(int k=1; argv[i][k]; k++){
				for(size_t j=0; j<arraylen(options); j++){
					if(options[j].opt != argv[i][k])continue;
					if(options[j].func){
						if(i+1==argc){
							error("missing operand");
							exit(EXIT_FAILURE);
						}
						options[j].func(state,argv[i+1]);
					}
					break;
				}
			}
		}
	}
}

int main(int argc,char **argv){
	tld_state state;
	memset(&state,0,sizeof(state));
	parse_arg(&state,argc,argv);
	return 0;
}
