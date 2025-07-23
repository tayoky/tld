#ifndef _TLD_H
#define _TLD_H

#include <stdio.h>
#include <string.h>
#include <errno.h>

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
#define T_TWO_POINT   ':'
#define T_INTEGER     128
#define T_STR         129
#define T_SECTIONS    130
#define T_ENTRY       131

#define ARCH_I386     1
#define ARCH_X86_64   2
#define ARCH_AARCH64  3

#define FORMAT_ELF64  1
#define FORMAT_BINARY 2

typedef struct tld_section {
	char *name;
	size_t size;
	char *data;
} tld_section;

typedef struct tld_symbol {
	tld_section *section;
	size_t offset;
	size_t size;
} tld_symbol;

typedef struct tld_file {
	FILE *file;
	int type;
	size_t sections_count;
	tld_section *sections;
	char *name;
	size_t symbols_count;
	tld_symbol *symbols;
} tld_file;

typedef struct tld_state {
	FILE *script;
	tld_file *out;
	tld_file **in;
	size_t in_count;
	int arch;
	char *entry_name;
	size_t addr; //the value of the .
	token *unget;
	int line;
	int flags;
} tld_state;

#define FLAG_R 0x01

token *next_token(tld_state *);
void destroy_token(token *t);
const char *token_name(token *t);
void error(const char *fmt,...);

tld_file *tld_open_file(const char *path,const char *mode);
int tld_save_file(tld_file *file,int format,int arch);
void tld_close_file(tld_file *file);
int elf_load(tld_file *file);
int elf64_save(tld_file *file,int arch);

int linking(tld_state *state);

int glob_match(const char *patern,const char *str);
int glob_path_match(const char *patern,const char *str);

#define arraylen(array) (sizeof(array)/sizeof(*array))

// cute custom perror
#undef perror
#define perror(str) error("%s : %s",str,strerror(errno))

#endif
