/*
 * dbgprintf.h - Debug printf support
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

#ifndef _DBGPRINTF_H_
#define _DBGPRINTF_H_

#include <stdio.h>
#include <debug.h>

#ifdef _DEBUG
	#define D_PRINTF(args...)	printf(args)
#else
	#define D_PRINTF(args...)	do {} while (0)
#endif

#define A_PRINTF(format, args...) \
	do { \
	D_PRINTF(format, ## args); \
	scr_printf(format, ## args); \
	} while (0)

#endif /*_DBGPRINTF_H_*/
