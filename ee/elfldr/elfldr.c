/*
 * elfldr.c - ELF loader
 *
 * Copyright (C) 2009 jimmikaelkael <jimmikaelkael@wanadoo.fr>
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
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
#include <syscallnr.h>

char *erl_id = "elfldr";
#if 0
char *erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

#define GS_BGCOLOUR	*((vu32*)0x120000E0)

/* do not link to strcpy() from libc! */
#define __strcpy(dest, src) \
	strncpy(dest, src, strlen(src))

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

#define MAX_ARGS 15

static int g_argc;
static char *g_argv[1 + MAX_ARGS];
static char g_argbuf[256];

static void (*OldLoadExecPS2)(const char *filename, int argc, char *argv[]) = NULL;
extern void HookLoadExecPS2(const char *filename, int argc, char *argv[]);

/*
 * ELF loader function
 */
static void loadElf(void)
{
	int i, fd;	
	elf_header_t elf_header;
	elf_pheader_t elf_pheader;	

	ResetEE(0x7f);

	/* wipe user memory */
	for (i = 0x00100000; i < 0x02000000; i += 64) {
		__asm__ (
			"\tsq $0, 0(%0) \n"
			"\tsq $0, 16(%0) \n"
			"\tsq $0, 32(%0) \n"
			"\tsq $0, 48(%0) \n"
			:: "r" (i)
		);
	}

	/* clear scratchpad memory */
	memset((void*)0x70000000, 0, 16 * 1024);

	/* reset IOP */
	SifInitRpc(0);
	SifResetIop();
	SifInitRpc(0);

	FlushCache(0);
	FlushCache(2);

	/* reload modules */
	SifLoadFileInit();
	SifLoadModule("rom0:SIO2MAN", 0, 0);
	SifLoadModule("rom0:MCMAN", 0, 0);
	SifLoadModule("rom0:MCSERV", 0, 0);

	/* load the ELF manually */
	fioInit();
 	fd = open(g_argv[0], O_RDONLY);
 	if (fd < 0) {
		goto error; /* can't open file, exiting... */
 	}

	/* read ELF header */
	if (read(fd, &elf_header, sizeof(elf_header)) != sizeof(elf_header)) {
		close(fd);
		goto error; /* can't read header, exiting... */
	}

	/* check ELF magic */
	if ((*(u32*)elf_header.ident) != ELF_MAGIC) {
		close(fd);
		goto error; /* not an ELF file, exiting... */
	}

	/* copy loadable program segments to RAM */
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

	/* exit services */
	fioExit();
	SifLoadFileExit();
	SifExitIopHeap();
	SifExitRpc();

	FlushCache(0);
	FlushCache(2);

	/* finally, run game ELF ... */
	ExecPS2((void*)elf_header.entry, NULL, g_argc, g_argv);
error:
	GS_BGCOLOUR = 0xffffff; /* white screen: error */
	SleepThread();
}

/*
 * LoadExecPS2 replacement function. The real one is evil...
 * This function is called by the main ELF to start the game.
 */
void MyLoadExecPS2(const char *filename, int argc, char *argv[])
{
	char *p = g_argbuf;
	int i;

	/* copy args from main ELF */
	g_argc = argc > MAX_ARGS ? MAX_ARGS : argc;

	memset(g_argbuf, 0, sizeof(g_argbuf));

	__strcpy(p, filename);
	g_argv[0] = p;
	p += strlen(filename) + 1;
	g_argc++;

	for (i = 0; i < argc; i++) {
		__strcpy(p, argv[i]);
		g_argv[i + 1] = p;
		p += strlen(argv[i]);
	}

	/*
	 * ExecPS2() does the following for us:
	 * - do a soft EE peripheral reset
	 * - terminate all threads and delete all semaphores
	 * - set up ELF loader thread and run it
	 */ 
	ExecPS2(loadElf, NULL, 0, NULL);
}

/*
 * _init - Automatically invoked on ERL load.
 */
int _init(void)
{
	/* Hook syscall */
	OldLoadExecPS2 = GetSyscall(__NR_LoadExecPS2);
	SetSyscall(__NR_LoadExecPS2, HookLoadExecPS2);

	return 0;
}

/*
 * _fini - Automatically invoked on ERL unload.
 */
int _fini(void)
{
	/* Unhook syscall */
	SetSyscall(__NR_LoadExecPS2, OldLoadExecPS2);

	return 0;
}
