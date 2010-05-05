/*
 * skeleton.c - skeleton for IOP module
 *
 * Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
 */

#include <tamtypes.h>
#include "irx_imports.h"
#include "skeleton.h"

IRX_ID(SKELETON_MODNAME, SKELETON_VER_MAJ, SKELETON_VER_MIN);

#define M_PRINTF(format, args...) \
	printf(SKELETON_MODNAME ": " format, ## args)

static int g_init = 0;

struct irx_export_table _exp_skeleton;

int skeleton_init(int arg)
{
	if (g_init)
		return -1;

	/* do init work... */

	g_init = 1;

	return 0;
}

int skeleton_exit(void)
{
	if (!g_init)
		return -1;

	/* do exit work... */

	g_init = 0;

	return 0;
}

#ifdef SKELETON_RPC
#define SKELETON_RPC_ID 0x01234567

enum {
	SKELETON_RPC_INIT = 1,
	SKELETON_RPC_EXIT,
};

static SifRpcServerData_t g_rpc_sd __attribute__((aligned(64)));
static SifRpcDataQueue_t g_rpc_qd __attribute__((aligned(64)));
static u8 g_rpc_buf[SKELETON_BUF_MAX] __attribute__((aligned(64)));

static void *rpc_handler(int cmd, void *buf, int size)
{
	int ret = -1;

	switch (cmd) {
	case SKELETON_RPC_INIT:
		ret = skeleton_init(*(int*)buf);
		break;
	case SKELETON_RPC_EXIT:
		ret = skeleton_exit();
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
	SifRegisterRpc(&g_rpc_sd, SKELETON_RPC_ID, (SifRpcFunc_t)rpc_handler,
		&g_rpc_buf, NULL, NULL, &g_rpc_qd);
	SifRpcLoop(&g_rpc_qd);
}

static int start_rpc_server(void)
{
	iop_thread_t thread;
 	int tid, ret;

	thread.attr = TH_C;
	thread.option = 0;
	thread.thread = (void*)rpc_thread;
	thread.stacksize = 4 * 1024;
	thread.priority = 79;

	if ((tid = CreateThread(&thread)) <= 0) {
		M_PRINTF("Could not create RPC thread (%i)\n", tid);
		return -1;
	}

	if ((ret = StartThread(tid, NULL)) < 0) {
		M_PRINTF("Could not start RPC thread (%i)\n", ret);
		DeleteThread(tid);
		return -1;
	}

	return tid;
}
#endif /* SKELETON_RPC */

int _start(int argc, char *argv[])
{
	if (RegisterLibraryEntries(&_exp_skeleton) != 0) {
		M_PRINTF("Could not register exports.\n");
		return MODULE_NO_RESIDENT_END;
	}

#ifdef SKELETON_RPC
	SifInitRpc(0);
	if (start_rpc_server() < 0) {
		ReleaseLibraryEntries(&_exp_skeleton);
		return MODULE_NO_RESIDENT_END;
	}
#endif
	M_PRINTF("Module started.\n");

	return MODULE_RESIDENT_END;
}
