/*
 * mycdvd.h - CDVD wrapper and helper functions
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
 */

#ifndef _MYCDVD_H_
#define _MYCDVD_H_

#include <tamtypes.h>
#include <libcdvd.h>

#define CDVD_BLOCK	0
#define CDVD_NOBLOCK	1

#if 0
int cdOpenTray(void);
int cdCloseTray(void);
void cdToggleTray(void);
int cdIsPS2Game(void);
#endif
void _cdStandby(int mode);
void _cdStop(int mode);

/**
 * cdGetElf - Parse "cdrom0:\\SYSTEM.CNF;1" for ELF filename.
 * @elfname: ptr to where filename is written to
 * @return: 0: success, <0: error
 */
int cdGetElf(char *elfname);

/**
 * cdRunElf - Run a PS2 game from CD/DVD.
 * @return: does not return on success, -1: error
 */

int cdRunElf(void);

#endif /* _MYCDVD_H_ */
