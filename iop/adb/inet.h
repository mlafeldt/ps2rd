/*
 * inet.h - Advanced debugger
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

#ifndef _IOP_INET_H_
#define _IOP_INET_H_

#include <tamtypes.h>
#include <irx.h>

#include "adb.h"

u16 htons(u16);
u16 ntohs(u16);
u32 htonl(u32);
u32 ntohl(u32);
u16 inet_chksum(ip_hdr_t *, u16);
u16 inet_chksum_pseudo(void *, u32 *, u32 *, u16);

#endif /* _IOP_INET_H_ */
