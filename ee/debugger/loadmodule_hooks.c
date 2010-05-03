/*
 * loadmodule_hooks.c - _sceSifLoadModule hooking
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
#include "loadmodule_patterns.h"

#define GS_BGCOLOUR	*((vu32*)0x120000e0)

int patch_loadModule(void);

/* modules list to fake loading */
char *module_tab[3] = {
	"DEV9.IRX",
	"SMAP.IRX",
	NULL
};

/* _sceSifLoadModule prototype */
int (*_sceSifLoadModule)(const char *path, int arg_len, const char *args, int *modres, int fno);

/*
 * hook function for _sceSifLoadModule
 */
static int Hook_SifLoadModule(const char *path, int arg_len, const char *args, int *modres, int fno)
{
	int ret, i, j;
	char filename[1024];
	char **p = &module_tab[0];
	int fake_loadmodule = 0;

	while (*p) {
		/* turn path to upper case */
		for (i=0; i<strlen(path); i++) {
			if ((path[i] >= 0x61) && (path[i] <= 0x7a))
				filename[i] = path[i] - 0x20;
			else
				filename[i] = path[i];
		}
		filename[i] = '\0';

		/* scan for module name */
		for (i=0; i<strlen(filename); i++) {
			for (j=0; j<strlen(*p); j++) {
				char *ptr = (char *)*p;
				if (filename[i+j] != ptr[j])
					break;
			}
			if (j == strlen(*p))
				fake_loadmodule = 1;
		}

		p++;
	}

	if (fake_loadmodule) {
		if (modres)
			*modres = 0;
		ret = 0;
	}
	else {
		ret = _sceSifLoadModule(path, arg_len, args, modres, fno);
	}

	return ret;
}

/*
 * This function patch the _sceSifLoadModule calls
 */
int patch_loadModule(void)
{
	u8 *ptr = NULL;
	u32 memscope, inst, fncall;
	u32 pattern[1], mask[1];
	u32 start = 0x00100000;
	int pattern_found = 0;
	const pattern_t *pat;

	GS_BGCOLOUR = 0x00a5ff; /* Orange while _sceSifLoadModule pattern search */

	memscope = 0x01f00000 - start;

	/* First try to locate the orginal _sceSifLoadModule function */
	pat = _loadmodule_patterns;
	while (pat->seq) {
		ptr = find_pattern((u8*)start, memscope, pat);
		if (ptr)
			break;
		pat++;
	}

	if (!ptr) {
		GS_BGCOLOUR = 0x808080; /* Gray, pattern not found */
		return 0;
	}

 	GS_BGCOLOUR = 0xcbc0ff; /* Pink while _sceSifLoadModule patches */

	/* Save original _sceSifLoadModule ptr */
	_sceSifLoadModule = (void *)ptr;

	/* Retrieve _sceSifLoadModule call Instruction code */
	inst = 0x0c000000;
	inst |= 0x03ffffff & ((u32)ptr >> 2);

	/* Make pattern with function call code saved above */
	pattern[0] = inst;
	mask[0] = 0xffffffff;

	/* Get Hook_SifLoadModule call Instruction code */
	inst = 0x0c000000;
	inst |= 0x03ffffff & ((u32)Hook_SifLoadModule >> 2);

	/* Search & patch for calls to _sceSifLoadModule */
	ptr = (u8 *)start;
	while (ptr) {
		memscope = 0x01f00000 - (u32)ptr;

		ptr = find_pattern_with_mask(ptr, memscope, (u8 *)pattern, (u8 *)mask, sizeof(pattern));
		if (ptr) {
			pattern_found = 1;

			fncall = (u32)ptr;
			_sw(inst, fncall); /* overwrite the original _sceSifLoadModule function call with our function call */

			ptr += 8;
		}
	}

	if (!pattern_found)
		GS_BGCOLOUR = 0x808080; /* gray, pattern not found */
	else
		GS_BGCOLOUR = 0x000000; /* Black, done */

	return 1;
}
