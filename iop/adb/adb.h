/*
 * adb.h - Advanced debugger
 *
 * Copyright (C) 2009-2010 misfire <misfire@xploderfreax.de>
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

#ifndef _IOP_ADB_H_
#define _IOP_ADB_H_

#include <tamtypes.h>
#include <irx.h>

/* Module name and version */
#define ADB_MODNAME	"adb"
#define ADB_VER_MAJ	1
#define ADB_VER_MIN	0

/* RPC buffer size */
#define ADB_BUF_MAX	1024

int adb_init(int arg);
int adb_exit(void);

/* global parameters struct */
typedef struct {
	u32 ip_addr_dst;
	u32 ip_addr_src;
	u16 ip_port_remote;
	u16 ip_port_local;
} g_param_t;

/* IRX import defines */
#define adb_IMPORTS_start	DECLARE_IMPORT_TABLE(adb, ADB_VER_MAJ, ADB_VER_MIN)
#define adb_IMPORTS_end		END_IMPORT_TABLE

#define I_adb_init		DECLARE_IMPORT(4, adb_init)
#define I_adb_exit		DECLARE_IMPORT(5, adb_exit)

#endif /* _IOP_ADB_H_ */
