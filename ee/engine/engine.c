/*
 * engine.c - Low-level cheat engine
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
#include <syscallnr.h>

char *erl_id = "engine";
#if 0
char *erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

#ifdef _HOOK_9
extern void *HookSetupThread(void *gp, void *stack, s32 stack_size, void *args,
	void *root_func);
#endif

/*
 * _init - Automatically invoked on ERL load.
 */
int _init(void)
{
	/* TODO Hook syscall */

	return 0;
}

/*
 * _fini - Automatically invoked on ERL unload.
 */
int _fini(void)
{
	/* TODO Unhook syscall */

	return 0;
}
