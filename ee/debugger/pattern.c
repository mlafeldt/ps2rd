/*
 * pattern.c - pattern search
 *
 * Copyright (C) 2009-2010 jimmikaelkael <jimmikaelkael@wanadoo.fr>
 * Copyright (C) 2009-2010 misfire <misfire@xploderfreax.de>
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
#include "pattern.h"

/* XXX I'm sure this can be optimized. */
u8 *find_pattern_with_mask(u8 *buf, u32 bufsize, u8 *bytes, u8 *mask, u32 len)
{
	u32 i, j;

	for (i = 0; i < bufsize - len; i++) {
		for (j = 0; j < len; j++) {
			if ((buf[i + j] & mask[j]) != bytes[j])
				break;
		}
		if (j == len)
			return &buf[i];
	}

	return NULL;
}

void *find_pattern(const void *buf, int size, const pattern_t *pat)
{
	return find_pattern_with_mask((u8*)buf, size, (u8*)pat->seq, (u8*)pat->mask, pat->len);
}
