/*
 * EE side of remote debugger
 *
 * Copyright (C) 2009-2010 jimmikaelkael <jimmikaelkael@wanadoo.fr>
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
 *
 * $Id$
 */

#include <kernel.h>
#include <sifrpc.h>
#include <string.h>

#define CMD_NONE  		0x00
#define CMD_GETREMOTECMD  	0x06
#define CMD_SENDDATA	  	0x0e
#define CMD_ENDREPLY	  	0x0f

/* defines for communication with debugger module */
#define NTPBCMD_SEND_DUMP 		0x300

#define REMOTE_CMD_NONE			0x000
#define REMOTE_CMD_DUMP			0x100
#define REMOTE_CMD_HALT			0x201
#define REMOTE_CMD_RESUME		0x202
#define REMOTE_CMD_ADDMEMPATCHES	0x501
#define REMOTE_CMD_CLEARMEMPATCHES	0x502
#define REMOTE_CMD_ADDRAWCODES		0x601
#define REMOTE_CMD_CLEARRAWCODES	0x602

static SifRpcClientData_t rpcclient 	__attribute__((aligned(64)));
static int Rpc_Buffer[16] 		__attribute__((aligned(64)));

static struct { 	// size = 4096
	u16 cmd;	// 0
	u8 buf[4090];	// 2
	int size;	// 4092
} sendDataParam __attribute__((aligned(64)));

static struct { 	// size = 64
	u16 cmd;	// 0
	u8 buf[56];	// 2
	int size;	// 58
	u16 pad;	// 62
} getRemoteCmdParam __attribute__((aligned(64)));

/* stores command currently being executed on the iop */
static unsigned int currentCmd = 0;

static int RPCclient_Inited = 0;

/* to control Halted/Resumed game state */
static int haltState = 0;

/*
 * rpcNTPBinit: this function init NTPB RPC Client
 */
int rpcNTPBinit(void)
{
	int ret = 0;

	if (RPCclient_Inited)
		return -1;

	SifInitRpc(0);

	/* bind to rpc on iop */
	do {
		if ((ret = SifBindRpc(&rpcclient, 0x80000a10, 0)) < 0)
			return ret;

		if (rpcclient.server == NULL)
			nopdelay();

	} while (rpcclient.server == NULL);

	/* successfully inited */
	RPCclient_Inited = 1;

	return 1;
}

/*
 * rpcNTPBreset: this function resets NTPB RPC Client
 */
int rpcNTPBreset(void)
{
	RPCclient_Inited = 0;
	rpcclient.server = NULL;
	return 1;
}

/*
 * rpcNTPBgetRemoteCmd: get a NTPB request sent by client to server on IOP
 */
static int rpcNTPBgetRemoteCmd(u16 *cmd, u8 *buf, int *size, int rpc_mode)
{
	int ret = 0;

	/* check lib is inited */
	if (!RPCclient_Inited)
		return -1;

	if((ret = SifCallRpc(&rpcclient, CMD_GETREMOTECMD, rpc_mode, NULL, 0, &getRemoteCmdParam, sizeof(getRemoteCmdParam), NULL, NULL)) != 0) {
		return ret;
	}

	*cmd = getRemoteCmdParam.cmd;
	*size = getRemoteCmdParam.size;
	if (getRemoteCmdParam.size > 0)
		memcpy(buf, getRemoteCmdParam.buf, getRemoteCmdParam.size);

	currentCmd = CMD_GETREMOTECMD;
	*(int*)Rpc_Buffer = 1;

	return 1;
}

/*
 * rpcNTPBsendData: send datas to the PC Client
 */
static int rpcNTPBsendData(u16 cmd, u8 *buf, int size, int rpc_mode)
{
	int ret = 0;

	/* check lib is inited */
	if (!RPCclient_Inited)
		return -1;

	/* set global variables */
	sendDataParam.cmd = cmd;
	if (buf)
		memcpy(sendDataParam.buf, buf, size);
	else
		memset(sendDataParam.buf, 0, size);

	sendDataParam.size = size;

	if((ret = SifCallRpc(&rpcclient, CMD_SENDDATA, rpc_mode, &sendDataParam, sizeof(sendDataParam), Rpc_Buffer, 4, NULL, NULL)) != 0) {
		return ret;
	}

	currentCmd = CMD_SENDDATA;

	*(int*)Rpc_Buffer = 1;

	return 1;
}

/*
 * rpcNTPBEndReply: Notify the end of reply to the PC Client
 */
static int rpcNTPBEndReply(int rpc_mode)
{
	int ret = 0;

	/* check lib is inited */
	if (!RPCclient_Inited)
		return -1;

	if((ret = SifCallRpc(&rpcclient, CMD_ENDREPLY, rpc_mode, NULL, 0, Rpc_Buffer, 4, NULL, NULL)) != 0) {
		return ret;
	}

	currentCmd = CMD_ENDREPLY;

	*(int*)Rpc_Buffer = 1;

	return 1;
}

/*
 * rpcNTPBSync: Sync RPC
 */
static int rpcNTPBSync(int mode, int *cmd, int *result)
{
	int funcIsExecuting, i;

	/* check if any functions are registered */
	if (currentCmd == CMD_NONE)
		return -1;

	/* check if function is still processing */
	funcIsExecuting = SifCheckStatRpc(&rpcclient);

	/* if mode = 0, wait for function to finish */
	if (mode == 0) {
		while (SifCheckStatRpc(&rpcclient)) {
			for (i=0; i<100000; i++) ;
		}
		/* function has finished */
		funcIsExecuting = 0;
	}

	/* get the function that just finished */
	if (cmd)
		*cmd = currentCmd;

	/* if function is still processing, return 0 */
	if (funcIsExecuting == 1)
		return 0;

	/* function has finished, so clear last command */
	currentCmd = CMD_NONE;

	/* get result */
	if(result)
		*result = *(int*)Rpc_Buffer;

	return 1;
}

/*
 * read_mem: this function reads memory
 */
static int read_mem(void *addr, int size, void *buf)
{
	DIntr();
	ee_kmode_enter();

	memcpy((void *)buf, (void *)addr, size);

	ee_kmode_exit();
	EIntr();

	return 0;
}

/*
 * send_dump: this function send a dump to the client
 */
static int sendDump(u32 dump_start, u32 dump_end, u8 *buf, int buflen, int rpc_mode)
{
	int r, len, sndSize, dumpSize, dpos, rpos;

	len = dump_end - dump_start;

	/* reducing dump size to fit in buffer */
	if (len > buflen)
		dumpSize = buflen;
	else
		dumpSize = len;

	dpos = 0;
	while (dpos < len) {

		/* dump mem part */
		read_mem((void *)(dump_start + dpos), dumpSize, buf);

		/* reducing send size for rpc if needed */
		if (dumpSize > 4096)
			sndSize = 4096;
		else
			sndSize = dumpSize;

		/* sending dump part datas */
		rpos = 0;
		while (rpos < dumpSize) {
			rpcNTPBsendData(NTPBCMD_SEND_DUMP, &buf[rpos], sndSize, rpc_mode);
			rpcNTPBSync(0, NULL, &r);
			rpos += sndSize;
			if ((dumpSize - rpos) < 4096)
				sndSize = dumpSize - rpos;
		}

		dpos += dumpSize;
		if ((len - dpos) < buflen)
			dumpSize = len - dpos;
	}

	/* send end of reply message */
	rpcNTPBEndReply(rpc_mode);
	rpcNTPBSync(0, NULL, &r);

	return len;
}

/*
 * Retrieve a Request sent by the client and fill it
 */
int rpcNTPBexecCmd(u8 *buf, int buflen, int rpc_mode)
{
	u16 remote_cmd;
	int size;
	int ret;
	u8 cmd_buf[64];

	if (rpc_mode == -1)
		return 0;

	/* get the remote command by RPC */
	rpcNTPBgetRemoteCmd(&remote_cmd, cmd_buf, &size, rpc_mode);
	rpcNTPBSync(0, NULL, &ret);

	if (remote_cmd != REMOTE_CMD_NONE) {
		/* handle Dump requests */
		if (remote_cmd == REMOTE_CMD_DUMP) {
			sendDump(*((u32*)&cmd_buf[0]), *((u32*)&cmd_buf[4]),
				buf, buflen, rpc_mode);
		}
		/* handle Halt request */
		else if (remote_cmd == REMOTE_CMD_HALT) {
			rpcNTPBEndReply(rpc_mode);
			rpcNTPBSync(0, NULL, &ret);
			if (!haltState) {
				haltState = 1;
				while (haltState)
					rpcNTPBexecCmd(buf, buflen, rpc_mode);
			}
		}
		/* handle Resume request */
		else if (remote_cmd == REMOTE_CMD_RESUME) {
			rpcNTPBEndReply(rpc_mode);
			rpcNTPBSync(0, NULL, &ret);
			if (haltState) {
				haltState = 0;
			}
		}
		/* handle raw mem patches adding */
		else if (remote_cmd == REMOTE_CMD_ADDMEMPATCHES) {
			rpcNTPBEndReply(rpc_mode);
			rpcNTPBSync(0, NULL, &ret);
			/*
			 * TODO ...
			 */
		}
		/* handle raw mem patches clearing */
		else if (remote_cmd == REMOTE_CMD_CLEARMEMPATCHES) {
			rpcNTPBEndReply(rpc_mode);
			rpcNTPBSync(0, NULL, &ret);
			/*
			 * TODO ...
			 */
		}
		/* handle codes adding */
		else if (remote_cmd == REMOTE_CMD_ADDRAWCODES) {
			rpcNTPBEndReply(rpc_mode);
			rpcNTPBSync(0, NULL, &ret);
			/*
			 * TODO ...
			 */
		}
		/* handle codes clearing */
		else if (remote_cmd == REMOTE_CMD_CLEARRAWCODES) {
			rpcNTPBEndReply(rpc_mode);
			rpcNTPBSync(0, NULL, &ret);
			/*
			 * TODO ...
			 */
		}
	}

	return 1;
}
