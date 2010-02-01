/*
 * smap.h - Advanced debugger
 *
 * Copyright (C) 2009-2010 jimmikaelkael <jimmikaelkael@wanadoo.fr>
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

#ifndef _IOP_SMAP_H_
#define _IOP_SMAP_H_

#include <tamtypes.h>
#include <irx.h>

#include <intrman.h>
#include <thbase.h>
#include <dev9.h>
#include <sysclib.h>
#include <speedregs.h>
#include <dev9regs.h>
#include <smapregs.h>

int smap_init(u8 *eth_addr_src);
int smap_xmit(void *buf, int size);

#endif /* _IOP_SMAP_H_ */
