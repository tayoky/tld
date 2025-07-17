#include <stdio.h>
#include <stdlib.h>
#include "tld.h"

//actual linking

#define syntax_error(str) {error("linker script syntax error");}

int expect(FILE *file,int type){
	token *tok = next_token(file);
	int t = tok->type;
	destroy_token(tok);
	if(t != type){
		syntax_error();
		exit(EXIT_FAILURE);
	
	}
	return 0;
}

int parse_sections(tld_state *state){
	expect(state->script,'{');
	return 0;
}

int linking(tld_state *state){
	for(;;){
		token *tok = next_token(state->script);
		printf("%s\n",token_name(tok));
		switch(tok->type){
		case T_ENTRY:
			expect(state->script,'(');
			expect(state->script,T_STR);
			expect(state->script,')');
			break;
		case T_SECTIONS:
			//TODO :error handling
			parse_sections(state);
			break;
		case T_EOF:
			destroy_token(tok);
			goto finish;
		case T_NEWLINE:
			break;
		default:
			syntax_error("unexpected token");
			return -1;
		}
		destroy_token(tok);
	}
finish:
	return 0;
}
