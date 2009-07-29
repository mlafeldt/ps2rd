/*
 * util.c - Useful utility functions
 *
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
#include <loadfile.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sifrpc.h>
#include "mycdvd.h"
#include "mystring.h"
#include "myutil.h"
#include "dbgprintf.h"

/**
 * kmem_read - Reads data from kernel memory.
 * @addr: memory address to read from
 * @buf: buffer to read data into
 * @size: size of data to read
 * @return: number of bytes read
 */
u32 kmem_read(void *addr, void *buf, u32 size)
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
u32 kmem_write(void *addr, const void *buf, u32 size)
{
	DI();
	ee_kmode_enter();
	memcpy(addr, buf, size);
	ee_kmode_exit();
	EI();

	return size;
}

/**
 * hook_syscall - Hooks a system call.
 * @syscall: number of syscall to hook
 * @myhandler: ptr to syscall handler (vector)
 * @return: original vector, or NULL on error
 *
 * This function patches the syscall vector table to call @myhandler first, each
 * time the syscall @syscall is executed.  Also, it inserts a jump opcode at the
 * address of "j 0" inside @myhandler that branches to the default syscall
 * handler.
 */
void *hook_syscall(s32 syscall, const void *myhandler)
{
	void *vector;
	u32 *j_defhandler;

	if (myhandler == NULL)
		return NULL;

	/* Get vector from syscall vector table */
	if ((vector = GetSyscall(syscall)) == NULL)
		return NULL;

	/* Search for opcode "j 0" inside myhandler */
	j_defhandler = (u32*)myhandler;
	while (*j_defhandler != MAKE_J(0))
		j_defhandler++;

	/* Insert jump opcode to syscall's default handler */
	*j_defhandler = MAKE_J(vector);

	/* Patch default vector to call my handler first */
	SetSyscall(syscall, KSEG0(myhandler));

	D_PRINTF("Hooked syscall 0x%02x (old vector %08x, new %08x, j_defhandler @%08x)\n",
		syscall, (u32)vector, (u32)KSEG0(myhandler), (u32)j_defhandler);

	return vector;
}

/**
 * flush_caches - Flushes data and instruction caches.
 */
void flush_caches(void)
{
	FlushCache(0); /* Writeback data cache */
	FlushCache(2); /* Instruction cache */
}

/**
 * reset_iop - Resets the IOP.
 * @img: filename of IOP replacement image; may be NULL
 */
void reset_iop(const char *img)
{
	D_PRINTF("%s: IOP says goodbye...\n", __FUNCTION__);

	SifInitRpc(0);

	/* Exit RPC services */
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();

	SifIopReset(img, 0);

	while (!SifIopSync())
		;
	SifInitRpc(0);
}

/**
 * load_modules - Loads multiple IOP modules.
 * @modv: list of modules
 * @return: 0: success, <0: error
 */
int load_modules(const char **modv)
{
	int i = 0, ret;

	if (modv == NULL)
		return -1;

	while (modv[i] != NULL) {
		ret = SifLoadModule(modv[i], 0, NULL);
		if (ret < 0) {
			D_PRINTF("%s: Failed to load module: %s (%i)\n",
				__FUNCTION__, modv[i], ret);
			return -1;
		}
		i++;
	}

	D_PRINTF("%s: All modules loaded.\n", __FUNCTION__);
	return 0;
}

/**
 * set_dir_name - Strips non-directory suffix from a filename.
 * @filename: full file path
 * @return: 0: success, <0: error
 */
int set_dir_name(char *filename)
{
	int i;

	if (filename == NULL)
		return -1;

	i = last_chr_idx(filename, '/');
	if (i < 0) {
		i = last_chr_idx(filename, '\\');
		if (i < 0) {
			i = chr_idx(filename, ':');
			if (i < 0)
				return -2;
		}
	}

	filename[i+1] = '\0';
	return 0;
}

/**
 * get_base_name - Strips directory and suffix ";1" from a filename.
 * @full: full file path
 * @base: ptr to where stripped filename is written to
 * @return: ptr to stripped filename, or NULL on error
 *
 * NOTE: @full and @base may be identical.
 */
char *get_base_name(const char *full, char *base)
{
	const char *p;
	int i, len;

	if (full == NULL || base == NULL)
		return NULL;

	i = last_chr_idx(full, '/');
	if (i < 0) {
		i = last_chr_idx(full, '\\');
		if (i < 0)
			i = chr_idx(full, ':');
	}
	p = &full[++i];
	len = last_chr_idx(p, ';');
	if (len < 0)
		len = strlen(p);
	memmove(base, p, len);
	base[len] = '\0';

	return base;
}

/* Prefix of boot devices */
const char *g_bootdev_prefix[] = {
	"cdrom0:",
	"host:",
	"mass:"
	"mc0:",
	"mc1:",
	NULL
};

/**
 * get_bootdev - Get boot device.
 * @path: path information
 * @return: boot device
 */
enum bootdev get_bootdev(const char *path)
{
	int i = 0;

	while (g_bootdev_prefix[i] != NULL) {
		if (!strncmp(path, g_bootdev_prefix[i],
			strlen(g_bootdev_prefix[i])))
				return i;
		i++;
	}

	return BOOT_UNKN;
}

/**
 * file_exists - Checks if a file exists.
 * @filename: name of file to check
 * @return: 1: file exists, 0: no such file
 */
int file_exists(const char *filename)
{
	int fd;

	if (filename == NULL)
		return 0;

	fd = open(filename, O_RDONLY);

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
char *read_text_file(const char *filename, int maxsize)
{
	char *buf = NULL;
	int fd, filesize;

	if (filename == NULL)
		return NULL;

	/* Open file */
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		D_PRINTF("%s: Can't open text file %s\n",
			__FUNCTION__, filename);
		return NULL;
	}

	/* Process file size */
	filesize = lseek(fd, 0, SEEK_END);
	if (maxsize && filesize > maxsize) {
		D_PRINTF("%s: Text file too large: %i bytes, max: %i bytes\n",
			__FUNCTION__, filesize, maxsize);
		goto end;
	}

	/* Allocate memory for file contents */
	buf = (char*)malloc(filesize + 1);
	if (buf == NULL) {
		D_PRINTF("%s: Unable to allocate %i bytes\n",
			__FUNCTION__, filesize + 1);
		goto end;
	}

	/* Read from text file into buffer */
	if (filesize > 0) {
		lseek(fd, 0, SEEK_SET);
		if (read(fd, buf, filesize) != filesize) {
			D_PRINTF("%s: Can't read from text file %s\n",
				__FUNCTION__, filename);
			goto end;
		}
	}

	/* NUL-terminate buffer */
	buf[filesize] = '\0';
end:
	close(fd);
	return buf;
}

/**
 * upload_file - Uploads a file into memory.
 * @filename: name of file to upload
 * @addr: memory address to upload file to
 * @size: number of written bytes
 * @return: 0: success, <0: error
 */
int upload_file(const char *filename, u32 addr, int *size)
{
	static u8 buf[32 * 1024] __attribute__((aligned(64)));
	u8 *p = (u8*)addr;
	int fd, bytes_read;

	if (filename == NULL)
		return -1;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		D_PRINTF("%s: Can't open file %s\n", __FUNCTION__, filename);
		return -2;
	}

	while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
		memcpy(p, buf, bytes_read); /* or kmem_write() ... */
		p += bytes_read;
	}

	close(fd);

	if (size != NULL)
		*size = bytes_read;

	return 0;
}
