/*
 * eesync.c
 *
 * Copyright (C) 2009 jimmikaelkael <jimmikaelkael@wanadoo.fr>
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
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

#include <stdio.h>
#include <loadcore.h>
#include <sifman.h>
#include "loadcore_add.h"

#define MODNAME "SyncEE"
IRX_ID(MODNAME, 1, 1);

struct irx_export_table _exp_eesync;

/* functions prototypes */
int PostResetcb(void);

/*
 * module entry point
 */
int _start(int argc, char** argv)
{
	register int *r;
	register int bootmode;

	r = QueryBootMode(3);

	if (r) {
		bootmode = *((int *)r + 4);

		if ((bootmode & 1) || (bootmode & 2))
			return MODULE_NO_RESIDENT_END;
	}

	/* Register eesync dummy export table */
	if (RegisterLibraryEntries(&_exp_eesync) < 0)
		return MODULE_NO_RESIDENT_END;

	/* set a post reset callback on	PostResetcb func */
	SetPostResetcb(PostResetcb, 2, NULL);

	return MODULE_RESIDENT_END;
}

/*
 * PostResetcb - loadcore call this once modules finished to load
 */
int PostResetcb(void)
{
	sceSifSetSMFlag(0x40000);

	return 0;
}
