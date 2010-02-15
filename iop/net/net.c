/*
 * net.c - lightweight network library
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

#include <tamtypes.h>
#include "irx_imports.h"
#include "net.h"
#include "inet.h"
#include "eth.h"
#include "ip.h"
#include "udp.h"

IRX_ID(NET_MODNAME, NET_VER_MAJ, NET_VER_MIN);

#define M_PRINTF(format, args...) \
	printf(NET_MODNAME ": " format, ## args)

struct irx_export_table _exp_net;

static int _net_init = 0;

/*
 * net_init: Initialize network interface
 */
int net_init(u32 remote_ip_addr, u32 local_ip_addr)
{
	if (_net_init)
		return -1;

	/* init Ethernet */
	if (eth_init(remote_ip_addr, local_ip_addr) != 0)
		return -2;

	_net_init = 1;
	M_PRINTF("Ready.\n");

	return 0;
}

int net_exit(void)
{
	if (!_net_init)
		return -1;

	/* TODO */

	_net_init = 0;

	return 0;
}

int _start(int argc, char *argv[])
{
	if (RegisterLibraryEntries(&_exp_net) != 0) {
		M_PRINTF("Could not register exports.\n");
		return MODULE_NO_RESIDENT_END;
	}

	M_PRINTF("Module started.\n");
	return MODULE_RESIDENT_END;
}
