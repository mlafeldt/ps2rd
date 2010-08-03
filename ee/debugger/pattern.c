/*
 * Pattern searching
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
#include "pattern.h"

u32 *find_pattern_with_mask(const u32 *buf, int bufsize,
	const u32 *seq, const u32 *mask, int len)
{
	int i, j;

	len /= sizeof(u32);
	bufsize /= sizeof(u32);

	for (i = 0; i < bufsize - len; i++) {
		for (j = 0; j < len; j++) {
			if ((buf[i + j] & mask[j]) != seq[j])
				break;
		}
		if (j == len)
			return (u32*)&buf[i];
	}

	return NULL;
}

u32 *find_pattern(const u32 *buf, int bufsize, const pattern_t *pat)
{
	return find_pattern_with_mask(buf, bufsize,
		pat->seq, pat->mask, pat->len);
}
