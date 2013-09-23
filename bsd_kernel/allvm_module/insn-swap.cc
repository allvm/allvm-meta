/* Horizon bytecode compiler
 * Copyright (C) 2010 James Molloy
 *
 * Horizon is open source software, released under the terms of the Non-Profit
 * Open Software License 3.0. You should have received a copy of the
 * licensing information along with the source code distribution. If you
 * have not received a copy of the license, please refer to the Horizon
 * project website.
 *
 * Please note that if you modify this file, the license requires you to
 * ADD your name to the list of contributors. This boilerplate is not the
 * license itself; please refer to the copy of the license you have received
 * for complete terms.
 */

/**@file   insn-swap.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Thu Jun  3 17:31:05 2010
 * @brief  A utility that looks through an ELF file for code entries, then disassembles that code, looking for the given input sequence, replacing with
           output sequence. */

#include <llvm/Support/CommandLine.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

extern "C" {
#include <udis86.h>
}

using namespace llvm;

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input ELF file>"), cl::init("-"));

static cl::opt<std::string>
SearchSeq("s", cl::desc("Search sequence in hex"), cl::value_desc("search"));

static cl::opt<std::string>
ReplaceSeq("r", cl::desc("Replace sequence in hex"), cl::value_desc("replace"));

#define PACKED __attribute__((packed))

#define PT_LOAD 1

typedef uint64_t Elf_Addr;
typedef uint64_t Elf_Off;
typedef uint16_t Elf_Half;
typedef uint32_t Elf_Word;
typedef  int32_t Elf_Sword;
typedef uint64_t Elf_Xword;
typedef  int64_t Elf_Sxword;

struct ElfHeader_t {
	char     ident[16];
	Elf_Half type;
	Elf_Half machine;
	Elf_Word version;
	Elf_Addr entry;
	Elf_Off  phoff;
	Elf_Off  shoff;
	Elf_Word flags;
	Elf_Half ehsize;
	Elf_Half phentsize;
	Elf_Half phnum;
	Elf_Half shentsize;
	Elf_Half shnum;
	Elf_Half shstrndx;
} PACKED;

struct ElfProgramHeader_t {
	Elf_Word type;
	Elf_Word flags;
	Elf_Off offset;
	Elf_Addr vaddr;
	Elf_Addr paddr;
	Elf_Xword filesz;
	Elf_Xword memsz;
	Elf_Xword align;
} PACKED;

int main(int argc, char **argv) {
	cl::ParseCommandLineOptions(argc, argv, "horizon ...\n");

	/* Convert the search and replace strings to binary form. */
	char *str = (char*)SearchSeq.c_str();
	unsigned char search[32];
	unsigned int search_len = strlen(str)/2;
	for(unsigned int i = 0; i < strlen(str); i += 2) {
		char c = str[i+2];
		str[i+2] = 0;
		search[i/2] = (unsigned char)strtoul(&str[i], 0, 16);
		str[i+2] = c;
	}
	
	str = (char*)ReplaceSeq.c_str();
	unsigned char replace[32];
	unsigned int replace_len = strlen(str)/2;
	for(unsigned int i = 0; i < strlen(str); i += 2) {
		char c = str[i+2];
		str[i+2] = 0;
		replace[i/2] = (unsigned char)strtoul(&str[i], 0, 16);
		str[i+2] = c;
	}

	if(replace_len != search_len) {
		std::cerr << "error: Search and replace strings must be of equal lengths." << std::endl;
		return 1;
	}

	int filedes = open(InputFilename.c_str(), O_RDWR);
	if(filedes == -1) {
		std::cerr << "error: File not accessible: " << InputFilename << std::endl;
		return 1;
	}

	unsigned long len = lseek(filedes, 0, SEEK_END);
	lseek(filedes, 0, SEEK_SET);

	unsigned char *map = (unsigned char *)mmap(0, len, PROT_READ|PROT_WRITE, MAP_SHARED, filedes, 0);
	if(map == MAP_FAILED) {
		std::cerr << "error: mmap() failed." << std::endl;
		return 1;
	}

	ElfHeader_t *header = (ElfHeader_t*)map;
	if(strncmp(&header->ident[1], "ELF", 3) != 0) {
		std::cerr << "error: Wrong ELF magic number" << std::endl;
		return 1;
	}

	ElfProgramHeader_t *pheader = (ElfProgramHeader_t*) (map + header->phoff);
	bool found = false;
	for(int i = 0; i < header->phnum; i++) {
		if(pheader->type == PT_LOAD) {
			/* HACK: assume .text is always in the first LOAD section. Should check executable permission */
			found = true;
			break;
		}

		++pheader;
	}

	if(!found) {
		std::cerr << "error: No PT_LOAD segment found." << std::endl;
		return 1;
	}

	unsigned char *data = map + pheader->offset;
	unsigned long length = pheader->filesz;

	ud_t obj;

	ud_init(&obj);
	ud_set_mode(&obj, 64);
	ud_set_syntax(&obj, UD_SYN_ATT);
	ud_set_pc(&obj, pheader->vaddr);
	ud_set_input_buffer(&obj, data, length);

	int nreplacements = 0;

	unsigned long location = pheader->vaddr;
	/* sometimes towards the end of the .text section we get stuck because of
	   padding bytes. Deal with this. */
	unsigned int stuck = 10;
	while(location <= pheader->vaddr + length) {
		ud_disassemble(&obj);
		unsigned long location2 = ud_insn_off(&obj);
		
		if((location2-location == search_len) && (memcmp(&data[location-pheader->vaddr], search, location2-location) == 0)) {
			memcpy(&data[location-pheader->vaddr], replace, location2-location);
			nreplacements ++;
		}

		if(location == location2) {
			stuck--;
			if(stuck == 0) {
				break;
			}
		}
		location = location2;
	}

	if(munmap(map, len) == -1) {
		std::cerr << "error: munmap() failed." << std::endl;
		return 1;
	}

	/* Get the source and target insns for printing. */
	ud_set_input_buffer(&obj, search, search_len);
	ud_disassemble(&obj);
	char src[32];
	int i = 0, j = 0;
	bool space = false;
	while(ud_insn_asm(&obj)[i]) {
		switch(ud_insn_asm(&obj)[i]) {
		case ' ':
			space = true;
			break;
		case '\0':
			src[j] = '\0';
			break;
		default:
			if(space) {
				src[j++] = ' ';
			}
			src[j++] = ud_insn_asm(&obj)[i];
		}
		i++;
	}
	src[j] = '\0';

	ud_set_input_buffer(&obj, replace, replace_len);
	ud_disassemble(&obj);
	char repl[32];
	space = false;
	i = 0; j = 0;
	while(ud_insn_asm(&obj)[i]) {
		switch(ud_insn_asm(&obj)[i]) {
		case ' ':
			space = true;
			break;
		case '\0':
			repl[j] = '\0';
			break;
		default:
			if(space) {
				repl[j++] = ' ';
				space = false;
			}
			repl[j++] = ud_insn_asm(&obj)[i];
		}
		i++;
	}
	repl[j] = '\0';

	std::cout << "Performing instruction replacement '" << SearchSeq << "' (" << src << ") -> '" << ReplaceSeq << "' (" << repl << ") in ELF file " << InputFilename << " - " << nreplacements << " replacements made.\n";

	return 0;
}
