/*
 * engine.c - Low-level cheat engine
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

#include <tamtypes.h>
#include <kernel.h>
#include <syscallnr.h>

char *erl_id = "engine";
/*
 * Import GetSyscallHandler() and SetSyscall() from libkernel.erl. If libkernel
 * is not installed, the boot loader will provide those functions instead.
 */
#if 0
char *erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

#ifdef _HOOK_9
static void *(*OldSetupThread)(void *gp, void *stack, s32 stack_size,
	void *args, void *root_func) = NULL;
extern void *HookSetupThread(void *gp, void *stack, s32 stack_size,
	void *args, void *root_func);
extern u32 j_SetupThread;
#endif

#define KSEG0(x)	((void*)(((u32)(x)) | 0x80000000))
#define MAKE_J(addr)	(u32)(0x08000000 | (0x03FFFFFF & ((u32)addr >> 2)))

/*
 * _init - Automatically invoked on ERL load.
 */
int _init(void)
{
	/* Hook syscall */
#ifdef _HOOK_9
	OldSetupThread = GetSyscallHandler(__NR_SetupThread);
	j_SetupThread = MAKE_J(OldSetupThread);
	SetSyscall(__NR_SetupThread, KSEG0(HookSetupThread));
#endif
	return 0;
}

/*
 * _fini - Automatically invoked on ERL unload.
 */
int _fini(void)
{
	/* Unhook syscall */
#ifdef _HOOK_9
	SetSyscall(__NR_SetupThread, OldSetupThread);
#endif
	return 0;
}
