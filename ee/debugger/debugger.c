/*
 * debugger.c - EE side of remote debugger
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
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
#include <loadfile.h>
#include <sifdma.h>
#include <string.h>
#include <syscallnr.h>

/*
 * TODO: Eventually, this is where all the hacking magic happens:
 * - load our IOP modules on IOP reboot
 * - handle SIF RPC (send EE RAM to IOP etc.)
 * - manage code and hook list of cheat engine
 */

char *erl_id = "debugger";
char *erl_dependancies[] = {
//	"libkernel",
	NULL
};

#define GS_BGCOLOUR *((volatile unsigned long int*)0x120000e0)

int (*OldSifSetReg)(u32 register_num, int register_value) = NULL;

/*
 * Hook function for syscall SifSetReg().
 */
int HookSifSetReg(u32 register_num, int register_value)
{
	/* TODO: do magic here */
	return OldSifSetReg(register_num, register_value);
}

/*
 * _init - Automatically invoked on ERL load.
 */
int _init(void)
{
	/* Hook syscall */
	OldSifSetReg = GetSyscall(__NR_SifSetReg);
	SetSyscall(__NR_SifSetReg, HookSifSetReg);

	return 0;
}

/*
 * _fini - Automatically invoked on ERL unload.
 */
int _fini(void)
{
	/* Unhook syscall */
	SetSyscall(__NR_SifSetReg, OldSifSetReg);

	return 0;
}

/*
 * This function is constantly called by the cheat engine.
 */
int debugger_loop(void)
{
	return 0;
}

#if 0
typedef struct {
	u32	hash;
	u8	*addr;
	u32	size;
} irxent_t;

static u8 g_buf[80*1024] __attribute__((aligned(64)));
static u8 *g_irx_buf = g_buf;

/* TODO: make this configurable */
#define IRX_ADDR 0x80030000

static int load_module_from_kernel(u32 hash, u32 arg_len, const char *args)
{
	const irxent_t *irx_ptr = (irxent_t*)IRX_ADDR;
	int irxsize = 0, ret;

	DI();
	ee_kmode_enter();

	/*
	 * find module by hash and copy it to user memory
	 */
	while (irx_ptr->hash) {
		if (irx_ptr->hash == hash) {
			irxsize = irx_ptr->size;
			memcpy(g_irx_buf, irx_ptr->addr, irxsize);
			break;
		}
		irx_ptr++;
	}

	ee_kmode_exit();
	EI();

	/* not found */
	if (!irxsize)
		return -1;

	/* load module */
	SifExecModuleBuffer(g_irx_buf, irxsize, arg_len, args, &ret);
	return ret;
}
#endif
