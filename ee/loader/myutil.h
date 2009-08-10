/*
 * myutil.h - Useful utility functions
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

/* Boot devices */
enum bootdev {
	BOOT_CD,
	BOOT_HOST,
	BOOT_MASS,
	BOOT_MC0,
	BOOT_MC1,
	BOOT_UNKN
};

u32 kmem_read(void *addr, void *buf, u32 size);
u32 kmem_write(void *addr, const void *buf, u32 size);
void *hook_syscall(s32 syscall, const void *myhandler);
void flush_caches(void);
void reset_iop(const char *img);
int load_modules(const char **modv);
int set_dir_name(char *filename);
char *get_base_name(const char *full, char *base);
enum bootdev get_bootdev(const char *path);
int file_exists(const char *filename);
char *read_text_file(const char *filename, int maxsize);
int upload_file(const char *filename, u32 addr, int *size);

#endif /*_MYUTIL_H_*/
