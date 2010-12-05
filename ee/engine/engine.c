/*
 * Cheat engine
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
 *
 * $Id$
 */

#include <tamtypes.h>

#ifdef _HOOK_9
extern u32 maxhooks;
extern u32 numhooks;
extern u32 hooklist[];

int get_max_hooks(void)
{
	return maxhooks;
}

int get_num_hooks(void)
{
	return numhooks;
}

const u32 *get_hook_list(void)
{
	return hooklist;
}

int add_hook(u32 addr, u32 val)
{
	if(numhooks >= maxhooks)
		return -1;

	hooklist[numhooks * 2] = addr & 0x01FFFFFC;
	hooklist[numhooks * 2 + 1] = val;
	numhooks++;

	return 0;
}

void clear_hooks(void)
{
	int i;

	for (i = 0; i < numhooks * 2; i++)
		hooklist[i] = 0;

	numhooks = 0;
}
#endif /* _HOOK_9 */

extern u32 maxcodes;
extern u32 numcodes;
extern u32 codelist[];

int get_max_codes(void)
{
	return maxcodes;
}

void set_max_codes(int num)
{
	maxcodes = num;
}

int get_num_codes(void)
{
	return numcodes;
}

const u32 *get_code_list(void)
{
	return codelist;
}

int add_code(u32 addr, u32 val)
{
	if (numcodes >= maxcodes)
		return -1;

	codelist[numcodes * 2] = addr;
	codelist[numcodes * 2 + 1] = val;
	numcodes++;

	return 0;
}

void clear_codes(void)
{
	int i;

	for (i = 0; i < numcodes * 2; i++)
		codelist[i] = 0;

	numcodes = 0;
}

extern u32 maxcallbacks;
extern u32 callbacks[];

/* TODO register multiple callbacks */
int register_callback(void *func)
{
	callbacks[0] = (u32)func;

	return 0;
}
