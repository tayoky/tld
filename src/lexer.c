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
	TOK(T_NEWLINE    ,"\n"),
	TOK(T_OPEN_BRACK ,"{"),
	TOK(T_CLOSE_BRACK,"}"),
	TOK(T_OPEN_PAREN ,"("),
	TOK(T_CLOSE_PAREN,")"),
	TOK(T_EQUAL      ,"="),
	TOK(T_SEMI_COLON ,";"),
	TOK(T_STAR       ,"*"),
};

struct keyword keywords[] = {
	KEYWORD(T_SECTIONS,"SECTIONS"),
	KEYWORD(T_ENTRY   ,"ENTRY"),
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


token *next_token(FILE *file){
	token *new = malloc(sizeof(token));
	memset(new,0,sizeof(token));

	//if aready at the end return EOF
	int c = fgetc(file);
	if(c == EOF){
		new->type = T_EOF;
		return new;
	}

	//if blank just skip
	while(isblank(c) || c == '\n'){
		c = fgetc(file);
	}
	ungetc(c,file);

	int op = get_token(file);
	if(op < 0){
		new->type = T_STR;
		new->value = strdup("");
		size_t size = 1;
		int c;
		while((c = fgetc(file)) != EOF){
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
		ungetc(c,file);

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
	return new;
}

void destroy_token(token *t){
	free(t->value);
	free(t);
}
