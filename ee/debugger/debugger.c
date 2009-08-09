/*
 * debugger.c - EE side of remote debugger
 *
 * Copyright (C) 2009 jimmikaelkael <jimmikaelkael@wanadoo.fr>
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
#include <iopheap.h>
#include <loadfile.h>
#include <sifdma.h>
#include <sifrpc.h>
#include <string.h>
#include <syscallnr.h>

/*
 * TODO: Eventually, this is where all the hacking magic happens:
 * - load our IOP modules on IOP reboot
 * - handle SIF RPC (send EE RAM to IOP etc.)
 * - manage code and hook list of cheat engine
 */

char *erl_id = "debugger";
#if 0
char *erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

#define GS_BGCOLOUR	*((vu32*)0x120000e0)

extern void OrigSifSetReg(u32 register_num, int register_value);
extern void HookSifSetReg(u32 register_num, int register_value);
extern int __NR_OrigSifSetReg;

static int (*OldSifSetReg)(u32 register_num, int register_value) = NULL;
static int set_reg_hook = 0;
static int debugger_ready = 0;
static u8 g_buf[80*1024] __attribute__((aligned(64)));


typedef struct {
	u32	hash;
	u8	*addr;
	u32	size;
} irxent_t;

/* TODO: make this configurable */
#define IRX_ADDR	0x80030000

#define HASH_PS2DEV9	0x0768ace9
#define HASH_PS2IP	0x00776900
#define HASH_PS2SMAP	0x0769a3f0

/*
 * load_module_from_kernel - Load IOP module from kernel RAM.
 */
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
			memcpy(g_buf, irx_ptr->addr, irxsize);
			break;
		}
		irx_ptr++;
	}

	ee_kmode_exit();
	EI();

	/* not found */
	if (!irxsize)
		return -1;

	/* load module from buffer */
	SifExecModuleBuffer(g_buf, irxsize, arg_len, args, &ret);
	return ret;
}

/*
 * post_reboot_hook - Will be executed after IOP reboot to load our modules.
 */
static int post_reboot_hook(void)
{
	int ret;

	/* init services */
	GS_BGCOLOUR = 0xff0000;
	SifInitRpc(0);
	SifLoadFileInit();
	SifInitIopHeap();

	GS_BGCOLOUR = 0xffff00;

	/* load our modules from kernel */
	ret = load_module_from_kernel(HASH_PS2DEV9, 0, NULL);
	if (ret < 0)
		while (1) ;
	ret = load_module_from_kernel(HASH_PS2IP, 0, NULL);
	if (ret < 0)
		while (1) ;
	ret = load_module_from_kernel(HASH_PS2SMAP, 0, NULL);
	if (ret < 0)
		while (1) ;

	GS_BGCOLOUR = 0x0000ff;

	/* deinit services */
	SifExitIopHeap();
	SifLoadFileExit();	
	SifExitRpc();

	GS_BGCOLOUR = 0x000000;

#ifdef DISABLE_AFTER_IOPRESET
	_fini();
#endif
	return 1;
}

/*
 * NewSifSetReg - Replacement function for SifSetReg().
 */
void NewSifSetReg(u32 regnum, int regval)
{
	/* call original SifSetReg() */
	OrigSifSetReg(regnum, regval);

	/* catch IOP reboot */
	if (regnum == SIF_REG_SMFLAG && regval == 0x10000) {
		debugger_ready = 0;
		set_reg_hook = 4;
	}

	if (set_reg_hook) {
		set_reg_hook--;
		/* check if reboot is done */
		if (!set_reg_hook && regnum == 0x80000000 && !regval) {
			/* IOP sync */
			while (!(SifGetReg(SIF_REG_SMFLAG) & 0x40000))
				;
			/* load our modules */
			post_reboot_hook();
		}
	}
}

/*
 * _init - Automatically invoked on ERL load.
 */
int _init(void)
{
	/* Hook syscalls */
	OldSifSetReg = GetSyscall(__NR_SifSetReg);
	SetSyscall(__NR_SifSetReg, HookSifSetReg);
	SetSyscall(__NR_OrigSifSetReg, OldSifSetReg);

	return 0;
}

/*
 * _fini - Automatically invoked on ERL unload.
 */
int _fini(void)
{
	/* Unhook syscalls */
	SetSyscall(__NR_SifSetReg, OldSifSetReg);
	SetSyscall(__NR_OrigSifSetReg, 0);

	return 0;
}

/*
 * debugger_loop - Constantly called by the cheat engine.
 */
int debugger_loop(void)
{
	/* TODO: handle remote debugging here */
	return 0;
}
