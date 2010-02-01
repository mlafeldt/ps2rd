/*
 * adb.c - Advanced debugger
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
#include "adb.h"
#include "../net/ip.h"
#include "../net/udp.h"
#include "../net/inet.h"

IRX_ID(ADB_MODNAME, ADB_VER_MAJ, ADB_VER_MIN);

#define M_PRINTF(format, args...) \
	printf(ADB_MODNAME ": " format, ## args)

struct irx_export_table _exp_adb;

static g_param_t g_param = {
	.ip_addr_dst = IP_ADDR(192, 168, 0, 2), 	/* remote IP addr */
	.ip_addr_src = IP_ADDR(192, 168, 0, 10),	/* local IP addr */
	.ip_port_remote = IP_PORT(7410), 			/* remote port */
	.ip_port_local = IP_PORT(8340)				/* local port */
};

static u8 udp_rcvbuf[1474] __attribute__((aligned(64))); /* 1 MTU max(1514) - ETH/IP/UDP headers + 1 */

static int _send_cl = -1;
static int _recv_cl = -1;

static int _adb_init = 0;

int adb_init(int arg)
{
	if (_adb_init)
		return -1;

	if (udp_open(&_send_cl, g_param.ip_port_remote, UDP_ACTIVE_CONN) < 0) {
		M_PRINTF("Could not create send udp client\n");
		return -1;
	}

	if (udp_open(&_recv_cl, g_param.ip_port_local, UDP_PASSIVE_CONN) < 0) {
		M_PRINTF("Could not create recv udp client\n");
		return -1;
	}

	_adb_init = 1;
	M_PRINTF("Ready.\n");

	return 0;
}

int adb_exit(void)
{
	if (!_adb_init)
		return -1;

	udp_close(_send_cl);
	udp_close(_recv_cl);

	_adb_init = 0;

	return 0;
}

#ifdef ADB_RPC
#define ADB_RPC_ID 0x01234567

enum {
	ADB_RPC_INIT = 1,
	ADB_RPC_EXIT
};

static SifRpcServerData_t g_rpc_sd __attribute__((aligned(64)));
static SifRpcDataQueue_t g_rpc_qd __attribute__((aligned(64)));
static u8 g_rpc_buf[ADB_BUF_MAX] __attribute__((aligned(64)));

static void *rpc_handler(int cmd, void *buf, int size)
{
	int ret = -1;

	switch (cmd) {
	case ADB_RPC_INIT:
		ret = adb_init(*(int*)buf);
		break;
	case ADB_RPC_EXIT:
		ret = adb_exit();
		break;
	default:
		M_PRINTF("Unknown RPC function call.\n");
	}

	*((int*)buf) = ret;

	return buf;
}

static void rpc_thread(void *arg)
{
	M_PRINTF("RPC thread started.\n");

	SifInitRpc(0);
	SifSetRpcQueue(&g_rpc_qd, GetThreadId());
	SifRegisterRpc(&g_rpc_sd, ADB_RPC_ID, (SifRpcFunc_t)rpc_handler,
		&g_rpc_buf, NULL, NULL, &g_rpc_qd);
	SifRpcLoop(&g_rpc_qd);
}
#endif /* ADB_RPC */

/*
 * Wrapper function to create and start threads.
 */
static int __start_thread(void *func, u32 stacksize, u32 priority)
{
	iop_thread_t thread;
 	int tid, ret;

	thread.attr = TH_C;
	thread.option = 0;
	thread.thread = func;
	thread.stacksize = stacksize;
	thread.priority = priority;

	if ((tid = CreateThread(&thread)) <= 0) {
		M_PRINTF("Could not create thread (%i)\n", tid);
		return -1;
	}

	if ((ret = StartThread(tid, NULL)) < 0) {
		M_PRINTF("Could not start thread (%i)\n", ret);
		DeleteThread(tid);
		return -1;
	}

	return tid;
}

/*
 * main server thread
 */
void server_thread(void *args)
{
	int pktsize;

	if (!adb_init(0)) {
		while (1) {
			pktsize = udp_recv(_recv_cl, udp_rcvbuf, sizeof(udp_rcvbuf));
			M_PRINTF("Got incoming UDP packet (%d bytes) at port %d\n", pktsize, ntohs(g_param.ip_port_local));

			/* output the received datas */
			udp_send(_send_cl, udp_rcvbuf, pktsize);
			M_PRINTF("Forwarded packet (%d bytes) to port %d\n", pktsize, ntohs(g_param.ip_port_remote));
		}
	}

	ExitDeleteThread();
}

int _start(int argc, char *argv[])
{
	if (RegisterLibraryEntries(&_exp_adb) != 0) {
		M_PRINTF("Could not register exports.\n");
		return MODULE_NO_RESIDENT_END;
	}

#ifdef ADB_RPC
	SifInitRpc(0);
	if (__start_thread(rpc_thread, 4*1024, 0x68) < 0)
		goto error;
#endif
	if (__start_thread(server_thread, 4*1024, 0x64) < 0)
		goto error;

	M_PRINTF("Module started.\n");
	return MODULE_RESIDENT_END;
error:
	ReleaseLibraryEntries(&_exp_adb);
	return MODULE_NO_RESIDENT_END;
}
