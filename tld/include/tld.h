#ifndef _TLD_H
#define _TLD_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
//use same relocation type as elf, for simplicity
#include "elf.h"

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
#define T_SLASH       '/'
#define T_TWO_POINT   ':'
#define T_PLUS        '+'
#define T_MINUS       '-'
#define T_INTEGER     128
#define T_STR         129
#define T_SECTIONS    130
#define T_ENTRY       131
#define T_PHDR        132
#define T_TARGET      132
#define T_ALIGN       133
#define T_BLOCK       134
#define T_COM_START   135
#define T_COM_END     136
#define T_OUTPUT_FMT  137
#define T_OUTPUT      138
#define T_FILEHDR     139
#define T_AT          140
#define T_FLAGS       141

#define ARCH_I386     1
#define ARCH_X86_64   2
#define ARCH_AARCH64  3

#define FORMAT_AUTO   0
#define FORMAT_ELF64  1
#define FORMAT_ELF32  2
#define FORMAT_BINARY 3

typedef struct tld_reloc {
	struct tld_symbol *symbol;
	size_t addend;
	size_t offset;
	int type;
} tld_reloc;

typedef struct tld_section {
	char *name;
	size_t size;
	char *data;
	uintptr_t address;
	size_t relocs_count;
	tld_reloc *relocs;
	int flags;
} tld_section;

#define TLD_SEC_R     0x01
#define TLD_SEC_W     0x02
#define TLD_SEC_X     0x04
#define TLD_SEC_NOBIT 0x08

typedef struct tld_symbol {
	char *name;
	tld_section *section;
	size_t linked; //used to link the copy of the symbol in local table
	size_t offset;
	size_t size;
	int flags;
	int type;
} tld_symbol;

#define TLD_SYM_UNDEF  0x01
#define TLD_SYM_WEAK   0x02
#define TLD_SYM_LOCAL  0x04
#define TLD_SYM_IGNORE 0x08
#define TLD_SYM_COMMON 0x10
#define TLD_SYM_ABS    0x20

#define TLD_SYM_NOTYPE  0
#define TLD_SYM_FUNC    1
#define TLD_SYM_OBJECT  2
#define TLD_SYM_SECTION 3
#define TLD_SYM_FILE    4

typedef struct tld_phdr {
	int type;
	int flags;
} tld_phdr;

typedef struct tld_file {
	FILE *file;
	int type;
	size_t sections_count;
	tld_section *sections;
	char *name;
	size_t symbols_count;
	tld_symbol *symbols;
	size_t phdrs_count;
	tld_phdr *phdrs;
	size_t entry;
} tld_file;

typedef struct tld_linker_opt {
	char *name;
	size_t value;
} tld_linker_opt;

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
	int input_format;
	int output_format;
	tld_linker_opt *linker_opts;
	size_t linker_opts_count;
} tld_state;

#define FLAG_RELOC 0x01

token *next_token(tld_state *);
void destroy_token(token *t);
const char *token_name(token *t);
void error(const char *fmt,...);

tld_file *tld_open_file(const char *path,const char *mode);
int tld_save_file(tld_file *file,int format,int arch);
void tld_close_file(tld_file *file);
int elf_load(tld_file *file);
int elf64_load(tld_file *file);
int elf32_load(tld_file *file);
int elf64_save(tld_file *file,int arch);
int elf32_save(tld_file *file,int arch);
int bin_save(tld_file *file,int arch);
int str2format(const char *str);
tld_symbol *tld_get_sym(tld_file *file,const char *name);

int linking(tld_state *state);
void tld_apply_relocations(tld_file *file,int arch);

int glob_match(const char *patern,const char *str);
int glob_path_match(const char *patern,const char *str);

#define arraylen(array) (sizeof(array)/sizeof(*array))

// cute custom perror
#undef perror
#define perror(str) error("%s : %s",str,strerror(errno))

#endif
