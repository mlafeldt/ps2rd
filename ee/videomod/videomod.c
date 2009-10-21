/*
 * videomod.c - Video mode patcher
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 *
 * This file is part of ps2rd, the PS2 remote debugger.
 *
 * ps2rd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ps2rd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ps2rd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <syscallnr.h>

char *erl_id = "videomod";
/* Import GetSyscallHandler() and SetSyscall() from libkernel.erl. */
#if 0
char *erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

static void (*OldSetGsCrt)(s16 interlace, s16 pal_ntsc, s16 field) = NULL;
extern void HookSetGsCrt(s16 interlace, s16 pal_ntsc, s16 field);
extern u32 j_SetGsCrt;

#define KSEG0(x)	((void*)(((u32)(x)) | 0x80000000))
#define MAKE_J(addr)	(u32)(0x08000000 | (0x03FFFFFF & ((u32)addr >> 2)))

int _init(void)
{
	OldSetGsCrt = GetSyscallHandler(__NR_SetGsCrt);
	j_SetGsCrt = MAKE_J(OldSetGsCrt);
	SetSyscall(__NR_SetGsCrt, KSEG0(HookSetGsCrt));

	return 0;
}

int _fini(void)
{
	SetSyscall(__NR_SetGsCrt, OldSetGsCrt);

	return 0;
}
