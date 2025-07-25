#include "elf.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "tld.h"

#ifndef BITS
#define BITS 64
#endif

#define _TYPEDEF(bits,suffix) typedef Elf##bits##_##suffix Elf_##suffix;
#define TYPEDEF(bits) _TYPEDEF(bits,Ehdr)\
	_TYPEDEF(bits,Shdr)\
	_TYPEDEF(bits,Phdr)\
	_TYPEDEF(bits,Sym)\
	_TYPEDEF(bits,Rela)\
	_TYPEDEF(bits,Rel)

#if BITS == 64
TYPEDEF(64)
#define ELF_R_SYM(i)     ELF64_R_SYM(i)
#define ELF_R_TYPE(i)    ELF64_R_TYPE(i)
#define ELF_R_INFO(s,t)  ELF64_R_INFO(i,t)
#define ELF_ST_BIND(i)   ELF64_ST_BIND(i)
#define ELF_ST_TYPE(i)   ELF64_ST_TYPE(i)
#define ELF_ST_INFO(b,t) ELF64_ST_INFO(b,t)
#define ELFCLASSXX       ELFCLASS64
#define elfxx_load       elf64_load
#define elfxx_save       elf64_save
#elif BITS == 32
TYPEDEF(32)
#define ELF_R_SYM(i)     ELF32_R_SYM(i)
#define ELF_R_TYPE(i)    ELF32_R_TYPE(i)
#define ELF_R_INFO(s,t)  ELF32_R_INFO(i,t)
#define ELF_ST_BIND(i)   ELF32_ST_BIND(i)
#define ELF_ST_TYPE(i)   ELF32_ST_TYPE(i)
#define ELF_ST_INFO(b,t) ELF32_ST_INFO(b,t)
#define ELFCLASSXX       ELFCLASS32
#define elfxx_load       elf32_load
#define elfxx_save       elf32_save
#endif

int elfxx_load (tld_file *file){
	rewind(file->file);
	Elf_Ehdr header;
	if(!fread(&header,sizeof(header),1,file->file))return -1;
	errno = EINVAL;
	if(header.e_ident[EI_VERSION] != EV_CURRENT || header.e_ident[EI_CLASS] != ELFCLASSXX || header.e_type != ET_REL){
		error("file is not a %s bit elf reloctable",BITS);
		return -1;
	}

	Elf_Shdr shstrtab_header;
	fseek(file->file,header.e_shoff + header.e_shentsize * header.e_shstrndx,SEEK_SET);
	if(!fread(&shstrtab_header,sizeof(shstrtab_header),1,file->file))return -1;

	char *shstrtab = malloc(shstrtab_header.sh_size);
	if(fseek(file->file,shstrtab_header.sh_offset,SEEK_SET) < 0)return -1;
	if(!fread(shstrtab,shstrtab_header.sh_size,1,file->file))return -1;

	file->sections_count = header.e_shnum;
	file->sections = calloc(file->sections_count,sizeof(tld_section));

	for(size_t i=0; i<file->sections_count; i++){
		if(fseek(file->file,header.e_shoff + header.e_shentsize * i,SEEK_SET) < 0)return -1;
		Elf_Shdr sheader;
		if(!fread(&sheader,sizeof(sheader),1,file->file)){
			free(shstrtab);
			return -1;
		}
		printf("find section %s of size %lu\n",&shstrtab[sheader.sh_name],sheader.sh_size);
		file->sections[i].size = sheader.sh_size;
		file->sections[i].name = strdup(&shstrtab[sheader.sh_name]);
		file->sections[i].data = malloc(sheader.sh_size);
		if(sheader.sh_flags & SHF_ALLOC){
			file->sections[i].flags |= TLD_SEC_R;
		}
		if(sheader.sh_flags & SHF_WRITE){
			file->sections[i].flags |= TLD_SEC_W;
		}
		if(sheader.sh_flags & SHF_EXECINSTR){
			file->sections[i].flags |= TLD_SEC_X;
		}
		if(sheader.sh_type == SHT_NOBITS){
			file->sections[i].flags |= TLD_SEC_NOBIT;
			memset(file->sections[i].data,0,sheader.sh_size);
		} else {
			fseek(file->file,sheader.sh_offset,SEEK_SET);
			fread(file->sections[i].data,sheader.sh_size,1,file->file);
		}
		switch(sheader.sh_type){
		case SHT_SYMTAB:
			//if null entisize expect default size
			//to maintain compatibilty with older version of tld
			if(!sheader.sh_entsize)sheader.sh_entsize = sizeof(Elf_Sym);
			fseek(file->file,header.e_shoff + header.e_shentsize * sheader.sh_link,SEEK_SET);
			Elf_Shdr strtab_hdr;
			fread(&strtab_hdr,sizeof(strtab_hdr),1,file->file);
			char *strtab = malloc(strtab_hdr.sh_size);
			fseek(file->file,strtab_hdr.sh_offset,SEEK_SET);
			fread(strtab,strtab_hdr.sh_size,1,file->file);

			file->symbols = realloc(file->symbols,(file->symbols_count + sheader.sh_size/sheader.sh_entsize) * sizeof(tld_symbol));
			memset(&file->symbols[file->symbols_count],0,sheader.sh_size/sheader.sh_entsize * sizeof(tld_symbol));
			Elf_Sym *symbols = (Elf_Sym *)file->sections[i].data;
			for(size_t j=0; j < sheader.sh_size/sheader.sh_entsize; j++){
				file->symbols[file->symbols_count+j].name = strdup(strtab+symbols[j].st_name);
				file->symbols[file->symbols_count+j].offset = symbols[j].st_value;
				file->symbols[file->symbols_count+j].size = symbols[j].st_size;

				//symbol binsing
				switch(ELF_ST_BIND(symbols[j].st_info)){
				case STB_LOCAL:
					file->symbols[file->symbols_count+j].flags |= TLD_SYM_LOCAL;
					break;
				case STB_GLOBAL:
					break;
				case STB_WEAK:
					file->symbols[file->symbols_count+j].flags |= TLD_SYM_WEAK;
					break;
				}
					


				//symbol type
				switch(ELF_ST_TYPE(symbols[j].st_info)){
				case STT_OBJECT:
					file->symbols[file->symbols_count+j].type = TLD_SYM_OBJECT;
					break;
				case STT_SECTION:
					file->symbols[file->symbols_count+j].type = TLD_SYM_SECTION;
					break;
				case STT_FUNC:
					file->symbols[file->symbols_count+j].type = TLD_SYM_FUNC;
					break;
				default:
					file->symbols[file->symbols_count+j].type = TLD_SYM_NOTYPE;
					break;
				}
				switch(symbols[j].st_shndx){
				case SHN_UNDEF:
					file->symbols[file->symbols_count+j].flags |= TLD_SYM_UNDEF;
					continue;
				case SHN_ABS:
					file->symbols[file->symbols_count+j].flags |= TLD_SYM_ABS;
					continue;
				case SHN_COMMON:
					file->symbols[file->symbols_count+j].flags |= TLD_SYM_COMMON;
					continue;
				}

				if(symbols[j].st_shndx >= SHN_LOPROC){
					file->symbols[file->symbols_count+j].flags |= TLD_SYM_IGNORE;
				} else {

					file->symbols[file->symbols_count+j].section = &file->sections[symbols[j].st_shndx];
				}
			}
			file->symbols_count += sheader.sh_size/sheader.sh_entsize;
			free(strtab);
		}

	}
	//we load relocations separently cause relocation can before section
	//(in which case we don't know the section)
	for(size_t i=0; i<file->sections_count; i++){
		if(fseek(file->file,header.e_shoff + header.e_shentsize * i,SEEK_SET) < 0)return -1;
		Elf_Shdr sheader;
		if(!fread(&sheader,sizeof(sheader),1,file->file)){
			free(shstrtab);
			return -1;
		}
		if(sheader.sh_type != SHT_RELA && sheader.sh_type != SHT_REL)continue;
		//find the section it apply to
		tld_section *section = &file->sections[sheader.sh_info];
		section->relocs = calloc(sheader.sh_size/sheader.sh_entsize,sizeof(tld_reloc));
		section->relocs_count = sheader.sh_size/sheader.sh_entsize;

		//we don't need to reload the section it's already in memory
		if(sheader.sh_type == SHT_RELA){
			Elf_Rela *relocs = (void *)file->sections[i].data;
			for(size_t j=0; j<section->relocs_count; j++){
					section->relocs[j].offset = relocs[j].r_offset;
				section->relocs[j].addend = relocs[j].r_addend;
				section->relocs[j].type   = ELF_R_TYPE(relocs[j].r_info);
			//TODO : better way of finding symbol ?
				section->relocs[j].symbol = &file->symbols[ELF_R_SYM(relocs[j].r_info)];
				printf("find reloc of type %lu linked with %s\n",ELF_R_TYPE(relocs[j].r_info),section->relocs[j].symbol->name);
			}
		} else {
			Elf_Rel *relocs = (void *)file->sections[i].data;
			for(size_t j=0; j<section->relocs_count; j++){
					section->relocs[j].offset = relocs[j].r_offset;
				section->relocs[j].addend = 0;
				section->relocs[j].type   = ELF_R_TYPE(relocs[j].r_info);
			//TODO : better way of finding symbol ?
				section->relocs[j].symbol = &file->symbols[ELF_R_SYM(relocs[j].r_info)];
				printf("find reloc of type %lu linked with %s\n",ELF_R_TYPE(relocs[j].r_info),section->relocs[j].symbol->name);
			}
		}
	}

	for(size_t i=0; i<file->symbols_count; i++){
		if(file->symbols[i].flags & TLD_SYM_IGNORE)continue;
		if(file->symbols[i].flags & TLD_SYM_UNDEF){
			printf("find undefined symbol %s\n",file->symbols[i].name);
		} else if(file->symbols[i].flags & TLD_SYM_ABS){
			printf("find absolute symbol %s\n",file->symbols[i].name);
		} else {
			printf("find symbol %s in %s\n",file->symbols[i].name,file->symbols[i].section->name);
		}
	}
	return 0;
}

#define add_string(str) strtab = realloc(strtab,strtab_len + strlen(str) + 1);\
				 strcpy(&strtab[strtab_len],str);\
				 strtab_len += strlen(str) + 1;

int elfxx_save(tld_file *file,int arch){
	puts("save");
	rewind(file->file);
	Elf_Ehdr header;
	memset(&header,0,sizeof(header));
	memcpy(header.e_ident,ELFMAG,SELFMAG);
	header.e_ident[EI_VERSION] = EV_CURRENT;
	header.e_ident[EI_CLASS]   = ELFCLASSXX;
	header.e_ident[EI_DATA]    = ELFDATA2LSB;
	switch(arch){
	case ARCH_I386:
		header.e_machine = EM_386;
		break;
	case ARCH_X86_64:
		header.e_machine = EM_X86_64;
		break;
	case ARCH_AARCH64:
		header.e_machine = EM_AARCH64;
		break;
	}
	header.e_ehsize = sizeof(header);
	header.e_type = ET_EXEC;
	header.e_entry = file->entry;
	header.e_phentsize = sizeof(Elf_Phdr);
	header.e_phnum = file->sections_count + 1;
	header.e_phoff = sizeof(header);
	header.e_shentsize = sizeof(Elf_Shdr);
	header.e_shnum = file->sections_count + 3;
	header.e_shoff = sizeof(header) + sizeof(Elf_Phdr) * (file->sections_count + 1);
	header.e_shstrndx = file->sections_count + 2;
	if(!fwrite(&header,sizeof(header),1,file->file))return -1;


	//init strtab
	char *strtab = strdup("");
	size_t strtab_len = 1;

	//write phdr
	off_t offset = sizeof(header) + (sizeof(Elf_Phdr) + sizeof(Elf_Shdr)) * (file->sections_count + 1 ) + sizeof(Elf_Shdr) * 2;
	Elf_Phdr pheader;
	memset(&pheader,0,sizeof(pheader));
	fwrite(&pheader,sizeof(pheader),1,file->file);
	for(size_t i=0; i<file->sections_count; i++){
		memset(&pheader,0,sizeof(pheader));
		pheader.p_type = PT_LOAD;
		pheader.p_memsz = file->sections[i].size;
		pheader.p_filesz = pheader.p_memsz;
		pheader.p_offset = offset;
		pheader.p_vaddr = file->sections[i].address;
		pheader.p_flags = PF_R | PF_W | PF_X;
		offset += pheader.p_filesz;
		fwrite(&pheader,sizeof(pheader),1,file->file);
	}

	//write shdr
	offset = sizeof(header) + (sizeof(Elf_Phdr) + sizeof(Elf_Shdr)) * (file->sections_count + 1 ) + sizeof(Elf_Shdr) * 2;
	Elf_Shdr sheader;
	memset(&sheader,0,sizeof(sheader));
	fwrite(&sheader,sizeof(sheader),1,file->file);
	for(size_t i=0; i<file->sections_count; i++){
		Elf_Shdr sheader;
		memset(&sheader,0,sizeof(sheader));
		sheader.sh_type = SHT_PROGBITS;
		sheader.sh_size = file->sections[i].size;
		sheader.sh_name = strtab_len;
		add_string(file->sections[i].name);
		sheader.sh_offset = offset;
		offset += file->sections[i].size;
		fwrite(&sheader,sizeof(sheader),1,file->file);
	}

	//write symtab header
	memset(&sheader,0,sizeof(sheader));
	sheader.sh_type = SHT_SYMTAB;
	sheader.sh_offset = offset;
	sheader.sh_info = 1024;
	sheader.sh_entsize = sizeof(Elf_Sym);
	sheader.sh_name = strtab_len;
	sheader.sh_link = header.e_shstrndx,
	add_string(".symtab");
	sheader.sh_size = (file->symbols_count + 1) * sizeof(Elf_Sym);
	offset += (file->symbols_count + 1)* sizeof(Elf_Sym);
	fwrite(&sheader,sizeof(sheader),1,file->file);

	//write stub strtab
	fwrite(&sheader,sizeof(sheader),1,file->file);

	//write sections
	for(size_t i=0; i<file->sections_count; i++){
		fwrite(file->sections[i].data,file->sections[i].size,1,file->file);
	}

	//write symtab content
	Elf_Sym sym = {
		.st_shndx = SHN_UNDEF,
	};
	fwrite(&sym,sizeof(sym),1,file->file);
	for(size_t i=0; i<file->symbols_count; i++){
		Elf_Sym sym = {
			.st_name = strtab_len,
			.st_value = file->symbols[i].offset,
			.st_size  = file->symbols[i].size,
			.st_shndx = 0/*(file->symbols[i].section - file->sections) / sizeof(tld_section) + 1,*/
		};

		if(file->symbols[i].flags & TLD_SYM_ABS){
			sym.st_shndx = SHN_ABS;
		}
		int type;
		switch(file->symbols[i].type){
		case TLD_SYM_FUNC:
			type = STT_FUNC;
			break;
		case TLD_SYM_OBJECT:
			type = STT_OBJECT;
			break;
		case TLD_SYM_SECTION:
			type = STT_SECTION;
			break;
		default:
			type = STT_NOTYPE;
			break;
		}
		int bind;
		if(file->symbols[i].flags & TLD_SYM_WEAK){
			bind = STB_WEAK;
		} else if(file->symbols[i].flags & TLD_SYM_LOCAL){
			bind = STB_LOCAL;
		} else {
			bind = STB_GLOBAL;
		}
		sym.st_info = ELF_ST_INFO(bind,type);
		add_string(file->symbols[i].name);
		fwrite(&sym,sizeof(sym),1,file->file);
	}

	//write strtab header
	fseek(file->file,header.e_shoff + header.e_shentsize * header.e_shstrndx,SEEK_SET);
	memset(&sheader,0,sizeof(sheader));
	sheader.sh_type = SHT_STRTAB;
	sheader.sh_offset = offset;
	sheader.sh_name = strtab_len;
	add_string(".strtab");
	sheader.sh_size = strtab_len;
	fwrite(&sheader,sizeof(sheader),1,file->file);

	//write strtab content
	fseek(file->file,offset,SEEK_SET);
	fwrite(strtab,strtab_len,1,file->file);

	return 0;
}
