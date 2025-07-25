#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "tld.h"

//small lexer for C like langages
//tayoky 2025

struct tok {
	char *str;
	size_t len;
	int type;
};

struct keyword {
	char *str;
	int type;
};

#define TOK(t,n) {.type = t,.str = n,.len = sizeof(n)-1}
#define KEYWORD(t,s) {.type = t,.str = s}

//must be from bigger to smaller
struct tok tokens[] = {
	//TOK(T_COM_START  ,"/*"),
	//TOK(T_COM_END    ,"*/"),
	TOK(T_NEWLINE    ,"\n"),
	TOK(T_OPEN_BRACK ,"{"),
	TOK(T_CLOSE_BRACK,"}"),
	TOK(T_OPEN_PAREN ,"("),
	TOK(T_CLOSE_PAREN,")"),
	TOK(T_EQUAL      ,"="),
	TOK(T_SEMI_COLON ,";"),
	TOK(T_TWO_POINT  ,":"),
	TOK(T_PLUS       ,"+"),
	TOK(T_MINUS      ,"-"),
	//TOK(T_SLASH      ,"/"),
	//TOK(T_STAR       ,"*"),
};

struct keyword keywords[] = {
	KEYWORD(T_SECTIONS  ,"SECTIONS"),
	KEYWORD(T_ENTRY     ,"ENTRY"),
	KEYWORD(T_PHDR      ,"PHDRS"),
	KEYWORD(T_TARGET    ,"TARGET"),
	KEYWORD(T_ALIGN     ,"ALIGN"),
	KEYWORD(T_BLOCK     ,"BLOCK"),
	KEYWORD(T_OUTPUT_FMT,"OUTPUT_FORMAT"),
	KEYWORD(T_OUTPUT    ,"OUTPUT"),
	KEYWORD(T_FILEHDR   ,"FILEHDR"),
	KEYWORD(T_AT        ,"AT"),
	KEYWORD(T_FLAGS     ,"FLAGS"),
};

const char *token_name(token *t){
	switch(t->type){
	case T_EOF:
		return "<eof>";
	case T_STR:
		return "<string>";
	case T_NEWLINE:
		return "<newline>";
	case T_SPACE:
		return "<space>";
	case T_INTEGER:
		return "<integer>";
	}

	for(size_t i=0; i<arraylen(keywords); i++){
		if(keywords[i].type == t->type){
			static char buf[256];
			snprintf(buf,sizeof(buf),"<%s>",keywords[i].str);
			return buf;
		}
	}

	for(size_t i=0; i<arraylen(tokens); i++){
		if(tokens[i].type == t->type){
			return tokens[i].str;
		}
	}
	return "<unknow>";
}

static int get_token(FILE *file){
	int c = fgetc(file);
	int best_match = -1;
	size_t size = 1;
	char str[8];
	str[0] = c;
	str[1] = '\0';
	for(size_t i=0; i < arraylen(tokens); i++){
		if(size == tokens[i].len && !memcmp(str,tokens[i].str,tokens[i].len)){
			best_match = i;
			//if we find a \n no token as anything after a \n
			//so we can return
			if(c == '\n')return i;
			c = fgetc(file);
			if(c == EOF)return i;
			str[size] = c;
			size++;
			str[size] = '\0';
			i = 0;
		}
	}

	ungetc(c,file);
	return best_match;
}


token *next_token(tld_state *state){
	token *new = malloc(sizeof(token));
	memset(new,0,sizeof(token));

	//if blank just skip
	int c = fgetc(state->script);
	while(isblank(c) || c == '\n'){
		if(c == '\n')state->line++;
		c = fgetc(state->script);
	}

	//if aready at the end return EOF
	if(c == EOF){
		new->type = T_EOF;
		return new;
	} else {
		ungetc(c,state->script);
	}


	int op = get_token(state->script);
	if(op < 0){
		new->type = T_STR;
		new->value = strdup("");
		size_t size = 1;
		int c;
		while((c = fgetc(state->script)) != EOF){
			if(isblank(c))b: break;
			//check if we are at the start of a new op
			for(size_t i=0; i<arraylen(tokens); i++){
				if(c == tokens[i].str[0])goto b;
			}
			size++;
			new->value = realloc(new->value,size);
			new->value[size-2] = c;
			new->value[size-1] = '\0';
		}
		ungetc(c,state->script);

		//check : is it a integer or a keyword ?
		if(isdigit(new->value[0])){
			//integer
			char *end;
			new->type = T_INTEGER;
			new->integer = strtol(new->value,&end,0);
			if(end == new->value){
				error("linker script syntax error : invalid integer");
				exit(EXIT_FAILURE);
			}
			switch(*end){
			case 'K':
				new->integer *= 1024;
				break;
			case 'M':
				new->integer *= 1024 * 1024;
				break;
			}
		} else {
			for(size_t i=0; i<arraylen(keywords); i++){
				if(!strcmp(new->value,keywords[i].str)){
					new->type = keywords[i].type;
					break;
				}
			}
		}
	} else {
		new->type = tokens[op].type;
	}
	//puts(token_name(new));
	return new;
}

void destroy_token(token *t){
	free(t->value);
	free(t);
}
