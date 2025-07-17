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

unsigned long parse_uint(FILE *file){
	//TODO : parse complex expression
	token *tok = next_token(file);
	if(tok->type != T_INTEGER){
		destroy_token(tok);
		syntax_error("expected integer");
	}
	unsigned long i = tok->integer;
	destroy_token(tok);
	return i;
}

int parse_sections(tld_state *state){
	expect(state->script,'{');
	for(;;){
		token *tok = next_token(state->script);
		switch(tok->type){
		case T_STR:;
			puts("h");
			//folowed by : is a section
			//folowed by = is an symbol
			//TODO : symbol modification
			token *next = next_token(state->script);
			switch(next->type){
			case ':':
				//TODO
				break;
			case '=':;
				unsigned long i = parse_uint(state->script);
				expect(state->script,';');
				if(!strcmp(tok->value,".")){
					if(i < state->addr){
						error(". value cannot go backward");
						exit(EXIT_FAILURE);
					}
					state->addr = i;
				} else {
					//TODO : create a symbols
				}
				break;
			default:
				syntax_error();
				break;
			}
			break;
		case '}':
			destroy_token(tok);
			return 0;
		}
		destroy_token(tok);
	}
	return 0;
}

int linking(tld_state *state){
	for(;;){
		token *tok = next_token(state->script);
		switch(tok->type){
		case T_ENTRY:
			expect(state->script,'(');
			token *entry = next_token(state->script);
			if(entry->type != T_STR)syntax_error();
			state->entry_name = strdup(entry->value);
			destroy_token(entry);
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
