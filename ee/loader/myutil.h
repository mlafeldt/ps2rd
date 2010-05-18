/*
 * myutil.h - Useful utility functions
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
 */

#ifndef _MYUTIL_H_
#define _MYUTIL_H_

#define NUM_SYSCALLS	256

#define KSEG0(x)	((void*)(((u32)(x)) | 0x80000000))
#define MAKE_J(addr)	(u32)(0x08000000 | (0x03FFFFFF & ((u32)addr >> 2)))
#define MAKE_JAL(addr)	(u32)(0x0C000000 | (0x03FFFFFF & ((u32)addr >> 2)))
#define MAKE_ORI(rt, rs, imm) \
			(u32)((0x0D << 26) | (rs << 21) | (rt << 16) | (imm))
#define J_TARGET(j)	(u32)((0x03FFFFFF & (u32)j) << 2)

#define JR_RA		0x03E00008
#define NOP		0x00000000

/* Devices returned by get_dev() */
enum dev {
	DEV_CD,
	DEV_HOST,
	DEV_MASS,
	DEV_MC0,
	DEV_MC1,
	DEV_UNKN
};

/**
 * kmem_read - Reads data from kernel memory.
 * @addr: memory address to read from
 * @buf: buffer to read data into
 * @size: size of data to read
 * @return: number of bytes read
 */
u32 kmem_read(void *addr, void *buf, u32 size);

/**
 * kmem_write - Writes data to kernel memory.
 * @addr: memory address to write to
 * @buf: buffer with data to write
 * @size: size of data to write
 * @return: number of bytes written
 */
u32 kmem_write(void *addr, const void *buf, u32 size);

/**
 * flush_caches - Flushes data and instruction caches.
 */
void flush_caches(void);

/**
 * install_debug_handler - Install debug exception handler.
 * @handler: handler function
 */
void install_debug_handler(const void *handler);

/**
 * reset_iop - Resets the IOP.
 * @img: filename of IOP replacement image; may be NULL
 */
void reset_iop(const char *img);

/**
 * load_modules - Loads multiple IOP modules.
 * @modv: list of modules
 * @return: 0: success, <0: error
 */
int load_modules(const char **modv);

/**
 * set_dir_name - Strips non-directory suffix from a filename.
 * @filename: full file path
 * @return: 0: success, <0: error
 */
int set_dir_name(char *filename);

/**
 * get_base_name - Strips directory and suffix ";1" from a filename.
 * @full: full file path
 * @base: ptr to where stripped filename is written to
 * @return: ptr to stripped filename, or NULL on error
 *
 * NOTE: @full and @base may be identical.
 */
char *get_base_name(const char *full, char *base);

/**
 * get_dev - Get device from path.
 * @path: path information
 * @return: boot device
 */
enum dev get_dev(const char *path);

/**
 * file_exists - Checks if a file exists.
 * @filename: name of file to check
 * @return: 1: file exists, 0: no such file
 */
int file_exists(const char *filename);

/**
 * read_text_file - Reads text from a file into a buffer.
 * @filename: name of text file
 * @maxsize: max file size (0: arbitrary size)
 * @return: ptr to NUL-terminated text buffer, or NULL if an error occured
 */
char *read_text_file(const char *filename, int maxsize);

/**
 * upload_file - Uploads a file into memory.
 * @filename: name of file to upload
 * @addr: memory address to upload file to
 * @size: number of written bytes
 * @return: 0: success, <0: error
 */
int upload_file(const char *filename, u32 addr, int *size);

#endif /*_MYUTIL_H_*/
