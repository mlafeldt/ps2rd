/*
 * debugger.c - EE side of remote debugger
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
#include <sifdma.h>

/*
 * TODO: Eventually, this is where all the hacking magic happens:
 * - load our IOP modules on IOP reboot
 * - handle SIF RPC (send EE RAM to IOP etc.)
 * - manage code and hook list of cheat engine
 */

char * erl_id = "debugger";

/* We will resolve dependencies by statically linking in required sources. */
#if 0
char * erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

#define GS_BGCOLOUR *((volatile unsigned long int*)0x120000e0)

u32 (*OldSifSetDma)(SifDmaTransfer_t *sdd, s32 len) = NULL;
int (*OldSifSetReg)(u32 register_num, int register_value) = NULL;

/*
 * Hook function for syscall SifSetDma().
 */
u32 HookSifSetDma(SifDmaTransfer_t *sdd, s32 len)
{
	//GS_BGCOLOUR = 0x0000ff;
	/* TODO: do magic here */
	return OldSifSetDma(sdd, len);
}

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
	/* Hook syscalls */
	OldSifSetDma = GetSyscall(__NR_SifSetDma);
	SetSyscall(__NR_SifSetDma, HookSifSetDma);

	OldSifSetReg = GetSyscall(__NR_SifSetReg);
	SetSyscall(__NR_SifSetReg, HookSifSetReg);

	return 0;
}

/*
 * _fini - Automatically invoked on ERL unload.
 */
int _fini(void)
{
	/* Unhook syscalls */
	SetSyscall(__NR_SifSetDma, OldSifSetDma);
	SetSyscall(__NR_SifSetReg, OldSifSetReg);

	return 0;
}

/*
 * This function is constantly called by the cheat engine.
 */
int debugger_loop(void)
{
#if 0
	/* Emulate code 10B8DAFA 00003F00 */
	_sh(0x3F00, 0x00B8DAFA);
#endif
	return 0;
}
