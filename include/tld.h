#ifndef _TLD_H
#define _TLD_H

#include <stdio.h>
#include <string.h>
#include <errno.h>

typedef struct tld_state {
	FILE *script;
	FILE *out;
	int arch;
} tld_state;

typedef struct token {
	int type;
	char *value;
	int integer;
} token;

#define T_EOF         1
#define T_SPACE       ' '
#define T_NEWLINE     '\n'
#define T_OPEN_BRACK  '{'
#define T_CLOSE_BRACK '}'
#define T_OPEN_PAREN  '('
#define T_CLOSE_PAREN ')'
#define T_EQUAL       '='
#define T_SEMI_COLON  ';'
#define T_STAR        '*'
#define T_INTEGER     128
#define T_STR         129
#define T_SECTIONS    130

token *next_token(FILE *file);
void destroy_token(token *t);
const char *token_name(token *t);
void error(const char *fmt,...);

#define arraylen(array) (sizeof(array)/sizeof(*array))

// cute custom perror
#undef perror
#define perror(str) error("%s : %s",str,strerror(errno))

#endif
