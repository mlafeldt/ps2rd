/*
 * adb.c - Advanced debugger
 *
 * Copyright (C) 2009-2010 misfire <misfire@xploderfreax.de>
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
#include "udpcl.h"

IRX_ID(ADB_MODNAME, ADB_VER_MAJ, ADB_VER_MIN);

#define M_PRINTF(format, args...) \
	printf(ADB_MODNAME ": " format, ## args)

struct irx_export_table _exp_adb;

//static char *_local_addr  = "192.168.0.10";
static char *_remote_addr = "192.168.0.2";
static int _local_port  = 8340;
static int _remote_port = 7410;
//static int _log_port    = 7411;

static int _adb_init = 0;
static udpcl_t _send_cl, _recv_cl;
static u8 _msg_buf[1024] __attribute__((aligned(64)));

int adb_init(int arg)
{
	if (_adb_init)
		return -1;

	if (udpcl_create(&_send_cl, _remote_addr, _remote_port, 0) < 0) {
		M_PRINTF("Could not create send udp client\n");
		return -1;
	}

	if (udpcl_create(&_recv_cl, NULL, _local_port, UDPCL_F_LISTEN) < 0) {
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

	udpcl_destroy(&_send_cl);
	udpcl_destroy(&_recv_cl);

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
 * Handle client requests to the debugger server.
 */
static void server_thread(void *arg)
{
	int ret;

	M_PRINTF("Server thread started.\n");

	if (!adb_init(0)) {
		while (1) {
			ret = udpcl_wait(&_recv_cl, -1);
			printf("\nwait: %i\n", ret);

			ret = udpcl_receive(&_recv_cl, _msg_buf, sizeof(_msg_buf));
			printf("receive: %i\n", ret);

			if (ret > 0) {
				ret = udpcl_send(&_send_cl, _msg_buf, ret);
				printf("send: %i\n", ret);
			}
		}
	}

	ExitDeleteThread();
}

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
