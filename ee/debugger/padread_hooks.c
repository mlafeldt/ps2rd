/*
 * padread_hooks.c - scePadRead hooking
 *
 * Copyright (C) 2009-2010 jimmikaelkael <jimmikaelkael@wanadoo.fr>
 * Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
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
#include <string.h>

#include "pattern.h"
#include "padread_patterns.h"

/* from debugger.c */
extern int debugger_loop(void);

#define GS_BGCOLOUR	*((vu32*)0x120000e0)

/* for hook addresses */
#define HOOKS_BASE	 	0x000fff00
#define MAX_HOOKS 		30

typedef struct {
	u32 address;
	u32 opcode;
} auto_hook_t;

/* pad buttons stats structs */
struct padButtonStat {
    u8 ok;
    u8 mode;
    u16 btns;
} __attribute__((packed));

struct pad2ButtonStat {
    u16 btns;
} __attribute__((packed));

int patch_padRead(void);

/* scePadRead prototypes */
static int (*scePadRead)(int port, int slot, struct padButtonStat *data);
static int (*scePad2Read)(int socket, struct pad2ButtonStat *data);
static int scePadRead_style = 1;

/*
 * hook function for libpad scePadRead
 */
static int Hook_scePadRead(int port, int slot, struct padButtonStat *data)
{
	int ret;

	ret = scePadRead(port, slot, data);
	debugger_loop();

	return ret;
}

/*
 * hook function for libpad2 scePad2Read
 */
static int Hook_scePad2Read(int socket, struct pad2ButtonStat *data)
{
	int ret;

	ret = scePad2Read(socket, data);
	debugger_loop();

	return ret;
}

/*
 * clear_autohook_tab - clears hook adresses/opcode table
 */
void clear_autohook_tab(void)
{
	memset((void *)HOOKS_BASE, 0, sizeof(auto_hook_t) * (MAX_HOOKS+1));
}

/*
 * add_autohook - helper to build hook adresses/opcode table
 */
static int add_autohook(u32 hook_addr, u32 orig_opcode)
{
	int i = 0;
	auto_hook_t *auto_hook = (auto_hook_t *)HOOKS_BASE;

	/* search for existing hook address */
	while (auto_hook->address) {
		if (auto_hook->address == hook_addr)
			return 0;
		auto_hook++;
		i++;
	}

	/* check that we get enough room space to store address */
	if (i >= MAX_HOOKS)
		return 0;

	/* store hook address */
	auto_hook->address = hook_addr;

	/* save original opcode */
	auto_hook->opcode = orig_opcode;

	return 1;
}

/*
 * This function patch the padRead calls
 */
int patch_padRead(void)
{
	u32 *ptr = NULL;
	u32 memscope, inst, fncall;
	u32 pattern[1], mask[1];
	u32 start = 0x00100000;
	int pattern_found = 0;
	const pattern_t *pat;

	GS_BGCOLOUR = 0x800080; /* Purple while padRead pattern search */

	memscope = 0x01f00000 - start;

	/* First try to locate the orginal libpad's scePadRead function */
	pat = _padread_patterns;
	while (pat->seq) {
		ptr = find_pattern((u32*)start, memscope, pat);
		if (ptr) {
			scePadRead_style = pat->tag; /* tag tells the version */
			break;
		}
		pat++;
	}

	if (!ptr) {
		GS_BGCOLOUR = 0x808080; /* Gray, pattern not found */
		return 0;
	}

 	GS_BGCOLOUR = 0x008000; /* Green while padRead patches */

	/* Save original scePadRead ptr */
	if (scePadRead_style == 2)
		scePad2Read = (void *)ptr;
	else
		scePadRead = (void *)ptr;

	/* Retrieve scePadRead call Instruction code */
	inst = 0x0c000000;
	inst |= 0x03ffffff & ((u32)ptr >> 2);

	/* Make pattern with function call code saved above */
	pattern[0] = inst;
	mask[0] = 0xffffffff;

	/* Get Hook_scePadRead call Instruction code */
	if (scePadRead_style == 2) {
		inst = 0x0c000000;
		inst |= 0x03ffffff & ((u32)Hook_scePad2Read >> 2);
	}
	else {
		inst = 0x0c000000;
		inst |= 0x03ffffff & ((u32)Hook_scePadRead >> 2);
	}

	/* Search & patch for calls to scePadRead */
	ptr = (u32*)start;
	while (ptr) {
		memscope = 0x01f00000 - (u32)ptr;

		ptr = find_pattern_with_mask(ptr, memscope, pattern, mask, sizeof(pattern));
		if (ptr) {
			/* store hook address */
			add_autohook((u32)ptr, _lw((u32)ptr));

			pattern_found = 1;

			fncall = (u32)ptr;
			_sw(inst, fncall); /* overwrite the original scePadRead function call with our function call */

			ptr += 2;
		}
	}

	if (!pattern_found)
		GS_BGCOLOUR = 0x808080; /* gray, pattern not found */
	else
		GS_BGCOLOUR = 0x000000; /* Black, done */

	return 1;
}
