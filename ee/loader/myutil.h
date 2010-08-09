/*
 * Useful utility functions
 *
 * Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
 *
 * This file is part of PS2rd, the PS2 remote debugger.
 *
 * PS2rd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PS2rd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PS2rd.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#ifndef _MYUTIL_H_
#define _MYUTIL_H_

#include <tamtypes.h>
#include <kernel.h>
#include <loadfile.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sifrpc.h>
#include <string.h>
#include "mycdvd.h"
#include "myutil.h"
#include "dbgprintf.h"

#define NUM_SYSCALLS	256

#define KSEG0(x)	((void*)(((u32)(x)) | 0x80000000))
#define MAKE_J(addr)	(u32)(0x08000000 | (0x03FFFFFF & ((u32)addr >> 2)))
#define MAKE_JAL(addr)	(u32)(0x0C000000 | (0x03FFFFFF & ((u32)addr >> 2)))
#define MAKE_ORI(rt, rs, imm) \
			(u32)((0x0D << 26) | (rs << 21) | (rt << 16) | (imm))
#define J_TARGET(j)	(u32)((0x03FFFFFF & (u32)j) << 2)

#define JR_RA		0x03E00008
#define NOP		0x00000000

/**
 * kmem_read - Reads data from kernel memory.
 * @addr: memory address to read from
 * @buf: buffer to read data into
 * @size: size of data to read
 * @return: number of bytes read
 */
static inline u32 kmem_read(void *addr, void *buf, u32 size)
{
	DI();
	ee_kmode_enter();
	memcpy(buf, addr, size);
	ee_kmode_exit();
	EI();

	return size;
}

/**
 * kmem_write - Writes data to kernel memory.
 * @addr: memory address to write to
 * @buf: buffer with data to write
 * @size: size of data to write
 * @return: number of bytes written
 */
static inline u32 kmem_write(void *addr, const void *buf, u32 size)
{
	DI();
	ee_kmode_enter();
	memcpy(addr, buf, size);
	ee_kmode_exit();
	EI();

	return size;
}

/**
 * install_debug_handler - Install debug exception handler.
 * @handler: handler function
 */
static inline void install_debug_handler(const void *handler)
{
	u32 *p = (u32*)0x80000100;

	DI();
	ee_kmode_enter();
	p[0] = MAKE_J(handler);
	p[1] = 0;
	ee_kmode_exit();
	EI();
	FlushCache(0);
}

/**
 * reset_iop - Resets the IOP.
 * @img: filename of IOP replacement image; may be NULL
 */
static inline void reset_iop(const char *img)
{
	D_PRINTF("%s: IOP says goodbye...\n", __FUNCTION__);

	SifInitRpc(0);

	cdInit(CDVD_INIT_NOCHECK);
	cdInit(CDVD_INIT_EXIT);

	while (!SifIopReset(img, 0))
		;
	while (!SifIopSync())
		;

	/* exit services */
	fioExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();

	FlushCache(0);
	FlushCache(2);

	/* init services */
	SifInitRpc(0);
	SifLoadFileInit();
	SifInitIopHeap();
	fioInit();

	FlushCache(0);
	FlushCache(2);
}

/* Device returned by get_dev() */
enum dev_id {
	DEV_CD,
	DEV_HOST,
	DEV_MASS,
	DEV_MC0,
	DEV_MC1,
	DEV_UNKN
};

static const char *dev_prefix[] = {
	[DEV_CD]   = "cdrom0:",
	[DEV_HOST] = "host:",
	[DEV_MASS] = "mass:",
	[DEV_MC0]  = "mc0:",
	[DEV_MC1]  = "mc1:",
	[DEV_UNKN] = NULL
};

/**
 * get_dev - Get device from path.
 * @path: path
 * @return: device id
 */
static inline enum dev_id get_dev(const char *path)
{
	int i = 0;

	while (dev_prefix[i] != NULL) {
		if (!strncmp(path, dev_prefix[i], strlen(dev_prefix[i])))
			return i;
		i++;
	}

	return DEV_UNKN;
}

/*
 * Build argument vector from string @s.
 */
static inline void build_argv(const char *s, int *argc, char *argv[])
{
	static char argbuf[256];
	int max;
	char *tok;

	max = *argc;
	*argc = 0;

	memset(argv, 0, max*sizeof(argv[0]));
	memset(argbuf, 0, sizeof(argbuf));
	strncpy(argbuf, s, sizeof(argbuf)-1);

	tok = strtok(argbuf, "\t ");
	while (tok != NULL && *argc < max) {
		D_PRINTF("%s: argv[%i] = %s\n", __FUNCTION__, *argc, tok);
		argv[(*argc)++] = tok;
		tok = strtok(NULL, "\t ");
	}
}

/**
 * file_exists - Checks if a file exists.
 * @filename: name of file to check
 * @return: 1: file exists, 0: no such file
 */
static inline int file_exists(const char *filename)
{
	int fd = open(filename, O_RDONLY);

	if (fd < 0) {
		return 0;
	} else {
		close(fd);
		return 1;
	}
}

/**
 * read_text_file - Reads text from a file into a buffer.
 * @filename: name of text file
 * @maxsize: max file size (0: arbitrary size)
 * @return: ptr to NUL-terminated text buffer, or NULL if an error occured
 */
static inline char *read_text_file(const char *filename, int maxsize)
{
	char *buf = NULL;
	int fd, filesize;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "%s: Can't open text file %s\n",
				__FUNCTION__, filename);
		return NULL;
	}

	filesize = lseek(fd, 0, SEEK_END);
	if (maxsize && filesize > maxsize) {
		fprintf(stderr, "%s: Text file too large: %i bytes, max: %i bytes\n",
			__FUNCTION__, filesize, maxsize);
		goto end;
	}

	buf = malloc(filesize + 1);
	if (buf == NULL) {
		fprintf(stderr, "%s: Unable to allocate %i bytes\n",
			__FUNCTION__, filesize + 1);
		goto end;
	}

	if (filesize > 0) {
		lseek(fd, 0, SEEK_SET);
		if (read(fd, buf, filesize) != filesize) {
			fprintf(stderr, "%s: Can't read from text file %s\n",
				__FUNCTION__, filename);
			free(buf);
			buf = NULL;
			goto end;
		}
	}

	buf[filesize] = '\0';
end:
	close(fd);
	return buf;
}

/*
 * Wrapper to call nopdelay many times.
 */
static inline void delay(int delay)
{
	int i = 0;

	while (i++ < delay)
		nopdelay();
}

#endif /*_MYUTIL_H_*/
