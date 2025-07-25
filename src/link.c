#include <stdio.h>
#include <stdlib.h>
#include "tld.h"
#include "elf.h"

//actual linking

#define syntax_error(str) {error("%s : linker script syntax error at %d",__func__,state->line);}

token *get_token(tld_state *state){
	if(state->unget){
		token *tok = state->unget;
		state->unget = NULL;
		return tok;
	}
	token *tok = next_token(state);
	if(tok->type == T_COM_START){
		while(tok->type != T_COM_END){
			destroy_token(tok);
			tok = next_token(state);
			if(tok->type == T_EOF){
				syntax_error("expect */");
				exit(EXIT_FAILURE);
			}
		}
	}
	return tok;
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

int get_op_level(token *op){
	static int ops[5][5] = {
		//higger
		{'!','~',0},
		{'*','/','%',0},
		{'+','-',0},
		{'&',0},
		{'|',0},
		//lower
	};
	for(int i = 0; i<arraylen(ops); i++){
		for(int j=0;ops[i][j]; j++){
			if(op->type == ops[i][j]){
				return i;
			}
		}
	}
	return -1;
}
unsigned long parse_uint(tld_state *state);

static unsigned long parse_simple_uint(tld_state *state){
	unsigned long i;
	token *tok = get_token(state);
	switch(tok->type){
	case T_INTEGER:
		i = tok->integer;
		destroy_token(tok);
		return i;
	case T_STR:
		//get symbol value
		if(!strcmp(tok->value,".")){
			destroy_token(tok);
			return state->addr;
		}
		//TODO : get symbols value
		break;
	case T_ALIGN:
	case T_BLOCK:
		destroy_token(tok);
		expect(state,'(');
		i = parse_uint(state);
		expect(state,')');
		//TODO :check if a power of 2
		return (state->addr + i - 1) & ~(i-1);
	case '(':
		destroy_token(tok);
		i = parse_uint(state);
		expect(state,')');
		return i;
	default:
		destroy_token(tok);
		syntax_error("expected integer");
		exit(EXIT_FAILURE);
	}
	destroy_token(tok);
	return 0;
}


unsigned long parse_uint(tld_state *state){
	//TODO : parse complex expression
	unsigned long i = parse_simple_uint(state);
	for(;;){
		token *op = get_token(state);
		switch(op->type){
		case '+':
			i += parse_simple_uint(state);
			break;
		case '-':
			i -= parse_simple_uint(state);
			break;
		default:
			unget_token(state,op);
			return i;
		}
		destroy_token(op);
	}
	return i;
}

static tld_symbol *create_symbol(tld_file *file,const char *name){
	//first let see if it already exist
	//NOTE: skip test if name is null
	if(name[0])
	for(size_t i=0; i<file->symbols_count; i++){
		if(!strcmp(name,file->symbols[i].name)){
			//it exist but if it is a week symbol or is undefined
			//we can replace it
			if(file->symbols[i].flags & TLD_SYM_WEAK || file->symbols[i].flags & TLD_SYM_UNDEF){
				return &file->symbols[i];
			} else {
				error("redefinition of symbol %s",name);
				//maybee exit ?
				return &file->symbols[i];
			}
		}
	}

	//it don't exist
	//make some place
	file->symbols = realloc(file->symbols,(file->symbols_count+1) * sizeof(tld_symbol));
	tld_symbol *sym = memset(&file->symbols[file->symbols_count++],0,sizeof(tld_symbol));
	sym->name = strdup(name);
	return sym;
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
		tld_symbol *sym = create_symbol(state->out,name);
		sym->offset = i;
		sym->size = 0;
		sym->flags = TLD_SYM_ABS;
	}
	return 0;
}

static void append_section(tld_state *state,tld_file *input_file,tld_section *input,tld_section *output){
	//append relocations
	output->relocs = realloc(output->relocs,(output->relocs_count + input->relocs_count) * sizeof(tld_reloc));
	for(size_t i=0; i<input->relocs_count; i++){
		output->relocs[output->relocs_count+i].offset = input->relocs[i].offset + output->size;
		output->relocs[output->relocs_count+i].addend = input->relocs[i].addend;
		output->relocs[output->relocs_count+i].type   = input->relocs[i].type;
		output->relocs[output->relocs_count+i].symbol = input->relocs[i].symbol;
	}
	output->relocs_count += input->relocs_count;

	//append symbols
	for(size_t i=0; i<input_file->symbols_count; i++){
		if(input_file->symbols[i].section != input)continue;
		//the symbols is on the specified section
		//append it
		tld_symbol *src  = &input_file->symbols[i];
		tld_symbol *dest = create_symbol(state->out,src->name);
		dest->size = src->size;
		//should we put absolute or relative ??
		//absolute i guess ?....
		dest->offset = src->offset + output->address + output->size;
		dest->flags = src->flags;
		dest->type  = src->type;
		src->linked = dest-state->out->symbols;
	}

	//append section data
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
				append_section(state,state->in[i],&state->in[i]->sections[k],&state->out->sections[output-1]);
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
	state->out->sections[state->out->sections_count-1].address = state->addr;
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
			goto ret;
		}
		destroy_token(tok);
	}	
ret:
	state->addr += state->out->sections[state->out->sections_count-1].size;
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

//append all absolute symbols of a file
void append_abs(tld_state *state,tld_file *file){
	for(size_t i=0; i<file->symbols_count; i++){
		if(!(file->symbols[i].flags & TLD_SYM_ABS))continue;
		tld_symbol *sym = create_symbol(state->out,file->symbols[i].name);
		sym->size  = file->symbols[i].size;
		sym->offset = file->symbols[i].offset;
		sym->type   = file->symbols[i].type;
		sym->flags  = file->symbols[i].flags;
		file->symbols[i].linked = sym - state->out->symbols;
	}
}

char *get_string(tld_state *state){
	token *tok = get_token(state);
	if(tok->type != T_STR){
		syntax_error("expected string");
		exit(EXIT_FAILURE);
	}
	char *str = strdup(tok->value);
	destroy_token(tok);
	return str;
}

void parse_phdr(tld_state *state){
	expect(state,'{');
	for(;;){
		token *tok = get_token(state);
		if(tok->type == '}'){
			destroy_token(tok);
			return;
		} else {
			unget_token(state,tok);
		}
		char *name = get_string(state);
		char *type  = get_string(state);
		unsigned long flags = 0;
		tok = get_token(state);
		if(tok->type == T_FILEHDR){
			destroy_token(tok);
			tok = get_token(state);
		}
		if(tok->type == T_PHDR){
			destroy_token(tok);
			tok = get_token(state);
		}

		//TODO : AT maybee ?
		if(tok->type == T_FLAGS){
			destroy_token(tok);
			expect(state,'(');
			flags = parse_uint(state);
			expect(state,')');
			tok = get_token(state);
		}
		unget_token(state,tok);
		expect(state,';');

		int t;
		if(!strcmp(type,"PT_NULL")){
			t = PT_NULL;
		} else	if(!strcmp(type,"PT_LOAD")){
			t = PT_LOAD;
		} else	if(!strcmp(type,"PT_DYNAMIC")){
			t = PT_DYNAMIC;
		} else	if(!strcmp(type,"PT_INTERP")){
			t = PT_INTERP;
		} else	if(!strcmp(type,"PT_NOTE")){
			t = PT_NOTE;
		} else	if(!strcmp(type,"PT_SHLIB")){
			t = PT_SHLIB;
		} else	if(!strcmp(type,"PT_PHDR")){
			t = PT_PHDR;
		} else {
			syntax_error("invalid phdr type");
		}

		state->out->phdrs = realloc(state->out->phdrs,(state->out->phdrs_count + 1) * sizeof(tld_phdr));
		state->out->phdrs[state->out->phdrs_count].type = t;
		state->out->phdrs[state->out->phdrs_count].flags = flags;
		state->out->phdrs_count++;
	}
}

int linking(tld_state *state){
	for(size_t i=0; i<state->in_count; i++){
		append_abs(state,state->in[i]);
	}
	state->line = 1;
	for(;;){
		token *tok = get_token(state);
		switch(tok->type){
		case T_OUTPUT_FMT:
			expect(state,'(');
			token *fmt = get_token(state);
			if(fmt->type != T_STR)syntax_error();
			if(state->output_format == FORMAT_AUTO){
				state->output_format = str2format(fmt->value);
				if(state->output_format < 0){
					error("invalid output format : %s",fmt->value);
					exit(EXIT_FAILURE);
				}
			}
			destroy_token(fmt);
			expect(state,')');
			break;
		case T_ENTRY:
			expect(state,'(');
			char *entry = get_string(state);
			if(state->entry_name){
				free(entry);
			} else {
				state->entry_name = entry;
			}
			expect(state,')');
			break;
		case T_SECTIONS:
			//TODO :error handling
			parse_sections(state);
			break;
		case T_PHDR:
			parse_phdr(state);
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
