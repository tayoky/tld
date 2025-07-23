#include <stdio.h>
#include <stdlib.h>
#include "tld.h"

//actual linking

#define syntax_error(str) {error("%s : linker script syntax error at %d",__func__,state->line);}

token *get_token(tld_state *state){
	if(state->unget){
		token *tok = state->unget;
		state->unget = NULL;
		return tok;
	}
	return next_token(state);
}

void unget_token(tld_state *state,token *tok){
	state->unget = tok;
}

int expect(tld_state *state,int type){
	token *tok = get_token(state);
	int t = tok->type;
	destroy_token(tok);
	if(t != type){
		syntax_error();
		exit(EXIT_FAILURE);
	
	}
	return 0;
}

unsigned long parse_uint(tld_state *state){
	//TODO : parse complex expression
	token *tok = get_token(state);
	if(tok->type != T_INTEGER){
		destroy_token(tok);
		syntax_error("expected integer");
	}
	unsigned long i = tok->integer;
	destroy_token(tok);
	return i;
}

int parse_symbol(tld_state *state,const char *name){
	expect(state,'=');
	unsigned long i = parse_uint(state);
	expect(state,';');
	if(!strcmp(name,".")){
		if(i < state->addr){
			error(". value cannot go backward");
			exit(EXIT_FAILURE);
		}
		state->addr = i;
	} else {
		//TODO : create a symbols
	}
	return 0;
}

static void append_section(tld_state *state,tld_section *input,tld_section *output){
	//TODO : append symbols and relocations too
	output->data = realloc(output->data,output->size + input->size);
	memcpy(&output->data[output->size],input->data,input->size);
	output->size += input->size;
}

static void parse_input_section(tld_state *state,const char *input,int output){
	printf("add section of %s in %d\n",input,output);
	expect(state,'(');
	size_t sec_count = 0;
	char **sec = NULL;
	token *tok = get_token(state);
	while(tok->type != ')'){
		if(tok->type != T_STR){
			syntax_error("expected closing ')'");
			exit(EXIT_FAILURE);
		}
		sec = realloc(sec,sizeof(char*)*(++sec_count));
		sec[sec_count-1] = strdup(tok->value);
		destroy_token(tok);
		tok = get_token(state);
	}
	destroy_token(tok);

	for(size_t i=0; i<state->in_count; i++){
		if(!glob_path_match(input,state->in[i]->name))continue;
		for(size_t j=0; j<sec_count; j++){
			for(size_t k=0; k<state->in[i]->sections_count; k++){
				if(!glob_match(sec[j],state->in[i]->sections[k].name))continue;
				append_section(state,&state->in[i]->sections[k],&state->out->sections[output-1]);
				printf("add %s of %s\n",state->in[i]->sections[k].name,state->in[i]->name);
			}
		}
	}

	for(size_t i=0; i<sec_count; i++){
		free(sec[i]);
	}
	free(sec);
}

int parse_output_section(tld_state *state,const char *name){
	printf("parse output section %s\n",name);
	state->out->sections = realloc(state->out->sections,(++state->out->sections_count) * sizeof(tld_section));
	memset(&state->out->sections[state->out->sections_count-1],0,sizeof(tld_section));
	state->out->sections[state->out->sections_count-1].name = strdup(name);
	token *tok = get_token(state);
	if(tok->type != ':' && tok->type != '('){
		//address
		unget_token(state,tok);
		unsigned long addr = parse_uint(state);
		if(addr < state->addr){
			error(". value cannot go backward");
			exit(EXIT_FAILURE);
		}
		state->addr = addr;
		tok = get_token(state);
	}
	unget_token(state,tok);
	expect(state,':');
	expect(state,'{');
	for(;;){
		token *tok = get_token(state);
		switch(tok->type){
		case T_STR:;
			//folowed by = is an symbol
			//folowed by anything else is an input section
			//TODO : symbol modification
			token *next = get_token(state);
			unget_token(state,next);
			switch(next->type){
			case '=':
				parse_symbol(state,tok->value);
				break;
			case '(':
				parse_input_section(state,tok->value,state->out->sections_count);
				break;
			default:
				syntax_error("expected input section description");
				exit(EXIT_FAILURE);
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

int parse_sections(tld_state *state){
	expect(state,'{');
	for(;;){
		token *tok = get_token(state);
		switch(tok->type){
		case T_STR:;
			//folowed by = is an symbol
			//folowed by anything else is an output section
			//TODO : symbol modification
			token *next = get_token(state);
			unget_token(state,next);
			switch(next->type){
			case '=':
				parse_symbol(state,tok->value);
				break;
			default:
				parse_output_section(state,tok->value);
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
	state->line = 1;
	for(;;){
		token *tok = get_token(state);
		switch(tok->type){
		case T_ENTRY:
			expect(state,'(');
			token *entry = get_token(state);
			if(entry->type != T_STR)syntax_error();
			state->entry_name = strdup(entry->value);
			destroy_token(entry);
			expect(state,')');
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
