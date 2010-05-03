/*
 * pattern.h - pattern search
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

#ifndef _PATTERN_H_
#define _PATTERN_H_

#include <tamtypes.h>

typedef struct _pattern {
	const u32 *seq;
	const u32 *mask;
	int len;
	int tag;	
} pattern_t;

u32 *find_pattern_with_mask(const u32 *buf, int bufsize,
        const u32 *seq, const u32 *mask, int len);
u32 *find_pattern(const u32 *buf, int bufsize, const pattern_t *pat);

#endif /* _PATTERN_H_ */
