/*
 * netlog_rpc.c - simple RPC interface to netlog.irx
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 *
 * This file is part of Artemis, the PS2 game debugger.
 *
 * Artemis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Artemis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Artemis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tamtypes.h>
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include "netlog_rpc.h"

enum {
        NETLOG_RPC_INIT = 1,
        NETLOG_RPC_EXIT,
        NETLOG_RPC_SEND
};

typedef struct {
        char    ipaddr[16];
        u16     ipport;
} init_args_t;

static SifRpcClientData_t g_rpc_client __attribute__((aligned(64)));
static u8 g_rpc_buf[NETLOG_MAX_MSG] __attribute__((aligned(64)));
static int g_init = 0;


int netlog_init(const char *ipaddr, int ipport)
{
	init_args_t *iargs = (init_args_t*)g_rpc_buf;

	if (g_init)
		return -1;
	if (ipaddr == NULL || ipport == 0)
		return -1;

	SifInitRpc(0);

	while (1) {
		if (SifBindRpc(&g_rpc_client, NETLOG_RPC_ID, 0) < 0)
			return -1;
		if (g_rpc_client.server != NULL)
			break;
		nopdelay();
	}

	strncpy(iargs->ipaddr, ipaddr, 16);
	iargs->ipaddr[15] = '\0';
	iargs->ipport = ipport;

	if (SifCallRpc(&g_rpc_client, NETLOG_RPC_INIT, 0, g_rpc_buf,
		sizeof(init_args_t), g_rpc_buf, 4, NULL, NULL) < 0)
			return -1;
	g_init = 1;

	return *(int*)g_rpc_buf;
}

int netlog_exit(void)
{
	if (!g_init)
		return -1;

        if (SifCallRpc(&g_rpc_client, NETLOG_RPC_EXIT, 0, NULL, 0, g_rpc_buf, 4,
		NULL, NULL) < 0)
			return -1;

	g_rpc_client.server = NULL;
	g_init = 0;

	return *(int*)g_rpc_buf;
}

int netlog_send(const char *format, ...)
{
	va_list ap;

	if (!g_init)
		return -1;

	va_start(ap, format);
	vsnprintf(g_rpc_buf, NETLOG_MAX_MSG, format, ap);
	va_end(ap);

        if (SifCallRpc(&g_rpc_client, NETLOG_RPC_SEND, 0, g_rpc_buf,
		strlen(g_rpc_buf), g_rpc_buf, 4, NULL, NULL) < 0)
			return -1;

	return *(int*)g_rpc_buf;
}
