/*
 * ERL init/deinit
 *
 * Copyright (C) 2009-2010 jimmikaelkael <jimmikaelkael@wanadoo.fr>
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

#include <tamtypes.h>
#include <kernel.h>
#include <syscallnr.h>

char *erl_id = "elfldr";
#if 0
char *erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

static void (*OldLoadExecPS2)(const char *filename, int argc, char *argv[]) = NULL;
extern void HookLoadExecPS2(const char *filename, int argc, char *argv[]);

int __attribute__((section(".init"))) _init(void)
{
	/* hook syscall */
	OldLoadExecPS2 = GetSyscallHandler(__NR_LoadExecPS2);
	SetSyscall(__NR_LoadExecPS2, HookLoadExecPS2);

	return 0;
}

int __attribute__((section(".fini"))) _fini(void)
{
	/* unhook syscall */
	SetSyscall(__NR_LoadExecPS2, OldLoadExecPS2);

	return 0;
}
