/*
 * elfldr.c - Elf loader program
 *
 * Copyright (C) 2009 jimmikaelkael <jimmikaelkael@wanadoo.fr>
 *
 * This file is part of Artemis, the PS2 game debugger.
 *
 * Artemis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Artemis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Artemis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <string.h>
#include <stdio.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <fileio.h>
#include <io_common.h>

char *erl_id = "elfldr";
#if 0
char *erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

#define GS_BGCOLOUR *((volatile unsigned long int*)0x120000E0)
#define ELF_MAGIC	0x464c457f
#define ELF_PT_LOAD	1

typedef struct {
	u8	ident[16];
	u16	type;
	u16	machine;
	u32	version;
	u32	entry;
	u32	phoff;
	u32	shoff;
	u32	flags;
	u16	ehsize;
	u16	phentsize;
	u16	phnum;
	u16	shentsize;
	u16	shnum;
	u16	shstrndx;
} elf_header_t;

typedef struct {
	u32	type;
	u32	offset;
	void	*vaddr;
	u32	paddr;
	u32	filesz;
	u32	memsz;
	u32	flags;
	u32	align;
} elf_pheader_t;

/*
 * Elf loader function
 */
void NewLoadExecPS2(const char *filename, s32 num_args, char **args)
{
	/* Ok it's a shortcut to the real LoadExecPS2 but at least it prevent 
	 * user mem clear below 0x100000 (for this launch) and allow to swap disc 
	 */
	
	int i, fd;
	elf_header_t elf_header;
	elf_pheader_t elf_pheader;	
	
	SifInitRpc(0);	
	
	/* Clearing user mem, so better not to have anything valuable on stack */
	for (i = 0x100000; i < 0x02000000; i += 64) {
		asm (
			"\tsq $0, 0(%0) \n"
			"\tsq $0, 16(%0) \n"
			"\tsq $0, 32(%0) \n"
			"\tsq $0, 48(%0) \n"
			:: "r" (i)
		);
	}	
		
	/* Reset IOP */
	SifResetIop();
	SifInitRpc(0);
	
	/* Reload modules */
	SifLoadFileInit();
	SifLoadModule("rom0:SIO2MAN", 0, 0);
	SifLoadModule("rom0:MCMAN", 0, 0);
	SifLoadModule("rom0:MCSERV", 0, 0);
							
	/* We load the elf manually */
	fioInit();
 	fd = open(filename, O_RDONLY);
 	if (fd < 0) {
	 	/* can't open file, exiting... */
		goto error;
 	}
 
	if (!lseek(fd, 0, SEEK_END)) {
		close(fd);
		/* Zero size ? something wrong ! */
		goto error;
	}
	
	/* Read the Elf Header */
	lseek(fd, 0, SEEK_SET);
	read(fd, &elf_header, sizeof(elf_header));
	
	/* Check Elf Magic */
	if ( (*(u32*)elf_header.ident) != ELF_MAGIC) {
		close(fd);
		/* not an elf file */
		goto error;
	}
	
	/* Scan through the ELF's program headers and copy them into apropriate RAM
	 * section, then padd with zeros if needed.
	 */
	for (i = 0; i < elf_header.phnum; i++) {
		lseek(fd, elf_header.phoff+(i*sizeof(elf_pheader)), SEEK_SET);
		read(fd, &elf_pheader, sizeof(elf_pheader));
		if (elf_pheader.type != ELF_PT_LOAD)
			continue;

		lseek(fd, elf_pheader.offset, SEEK_SET);
		read(fd, elf_pheader.vaddr, elf_pheader.filesz);
		
		if (elf_pheader.memsz > elf_pheader.filesz)
			memset(elf_pheader.vaddr + elf_pheader.filesz, 0,
					elf_pheader.memsz - elf_pheader.filesz);
	}
	close(fd);
				
	fioExit();
	SifLoadFileExit();
	SifExitIopHeap();
	SifExitRpc();

	FlushCache(0);
	FlushCache(2);
				
	/* Execute... */
	ExecPS2((void *)elf_header.entry, 0, num_args, args);

error:
	GS_BGCOLOUR = 0xffffff; /* white screen: fatal error */
	while (1){;}		
}

/*
 * _init - Automatically invoked on ERL load.
 */
int _init(void)
{	
	return 0;
}

/*
 * _fini - Automatically invoked on ERL unload.
 */
int _fini(void)
{
	return 0;
}
