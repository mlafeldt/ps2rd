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

char *erl_id = "debugger";
#if 0
char *erl_dependancies[] = {
	"libkernel",
	"libpatches",
	NULL
};
#endif

extern void init_debugger_opts(void);
extern void clear_autohook_tab(void);

extern void HookSifSetReg(u32 register_num, int register_value);
extern u32 HookSifSetDma(SifDmaTransfer_t *sdd, s32 len);
extern u32 (*OldSifSetDma)(SifDmaTransfer_t *sdd, s32 len);
extern int (*OldSifSetReg)(u32 register_num, int register_value);

int __attribute__((section(".init"))) _init(void)
{
	init_debugger_opts();
	clear_autohook_tab();

	/* hook syscalls */
	OldSifSetDma = GetSyscallHandler(__NR_SifSetDma);
	SetSyscall(__NR_SifSetDma, HookSifSetDma);
	OldSifSetReg = GetSyscallHandler(__NR_SifSetReg);
	SetSyscall(__NR_SifSetReg, HookSifSetReg);

	return 0;
}

int __attribute__((section(".fini"))) _fini(void)
{
	/* unhook syscalls */
	SetSyscall(__NR_SifSetDma, OldSifSetDma);
	SetSyscall(__NR_SifSetReg, OldSifSetReg);

	return 0;
}
