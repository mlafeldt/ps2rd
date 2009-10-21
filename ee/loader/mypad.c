/*
 * mypad.c - Pad wrapper and helper functions
 *
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

#include <tamtypes.h>
#include "dbgprintf.h"
#include "mypad.h"

void padWaitReady(int port, int slot)
{
	int state, last_state;
	char str[16];

	state = padGetState(port, slot);
	last_state = -1;
	while ((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
		if (state != last_state) {
			padStateInt2String(state, str);
			D_PRINTF("Please wait, pad (%d/%d) is in state %s.\n",
				port+1, slot+1, str);
		}
		last_state = state;
		state = padGetState(port, slot);
	}

	/* Pad ever 'out of sync'? */
	if (last_state != -1)
		D_PRINTF("Pad (%d/%d) OK!\n", port+1, slot+1);
}
