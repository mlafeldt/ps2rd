/*
 * debugger.c - EE side of remote debugger
 *
 * Copyright (C) 2009 jimmikaelkael <jimmikaelkael@wanadoo.fr>
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
#include <kernel.h>
#include <iopheap.h>
#include <loadfile.h>
#include <sifdma.h>
#include <sifrpc.h>
#include <string.h>
#include <syscallnr.h>

/*
 * TODO: Eventually, this is where all the hacking magic happens:
 * - load our IOP modules on IOP reboot
 * - handle SIF RPC (send EE RAM to IOP etc.)
 * - manage code and hook list of cheat engine
 */

char *erl_id = "debugger";
#if 0
char *erl_dependancies[] = {
	"libkernel",
	NULL
};
#endif

/* padread_hooks.c function */
extern int patch_padRead(void);

/* debugger_rpc.c functions */
extern int rpcNTPBinit(void);
extern int rpcNTPBreset(void);
extern int rpcNTPBgetRemoteCmd(u16 *cmd, u8 *buf, int *size, int rpc_mode);
extern int rpcNTPBsendData(u16 cmd, u8 *buf, int size, int rpc_mode);
extern int rpcNTPBEndReply(int rpc_mode);
extern int rpcNTPBSync(int mode, int *cmd, int *result);

#define GS_BGCOLOUR	*((vu32*)0x120000e0)

extern void OrigSifSetReg(u32 register_num, int register_value);
extern void HookSifSetReg(u32 register_num, int register_value);
extern int __NR_OrigSifSetReg;

static int (*OldSifSetReg)(u32 register_num, int register_value) = NULL;
static int set_reg_hook = 0;
static int debugger_ready = 0;

/* libkernel reboot counter */
extern int _iop_reboot_count;

/* internal reboot counter */
static int iop_reboot_count = 0;

#define BUFSIZE 	80*1024
static u8 g_buf[BUFSIZE] __attribute__((aligned(64)));

/* RAM file table entry */
typedef struct {
	u32	hash;
	u8	*addr;
	u32	size;
} ramfile_t;

/* TODO: make this configurable */
#define IRX_ADDR	0x80030000

#define HASH_PS2DEV9	0x0768ace9
#define HASH_PS2IP	0x00776900
#define HASH_PS2SMAP	0x0769a3f0
#define HASH_DEBUGGER	0x0b9bdb62
#define HASH_NETLOG	0x074cb357

/* defines for communication with debugger module */
#define NTPBCMD_PRINT_EEDUMP 		0x301
#define NTPBCMD_PRINT_IOPDUMP		0x302
#define NTPBCMD_PRINT_KERNELDUMP 	0x303
#define NTPBCMD_PRINT_SCRATCHPADDUMP	0x304

#define REMOTE_CMD_NONE			0x000
#define REMOTE_CMD_DUMPEE		0x101
#define REMOTE_CMD_DUMPIOP		0x102
#define REMOTE_CMD_DUMPKERNEL		0x103
#define REMOTE_CMD_DUMPSCRATCHPAD	0x104
#define REMOTE_CMD_HALT			0x201
#define REMOTE_CMD_RESUME		0x202
#define REMOTE_CMD_ADDMEMPATCHES	0x501
#define REMOTE_CMD_CLEARMEMPATCHES	0x502
#define REMOTE_CMD_ADDRAWCODES		0x601
#define REMOTE_CMD_CLEARRAWCODES	0x602

#define PRINT_DUMP			0x300

#define EE_DUMP				1
#define IOP_DUMP			2
#define KERNEL_DUMP			3
#define SCRATCHPAD_DUMP		4

/* to control Halted/Resumed game state */
static int haltState = 0;

/* to control automatic hooking */
#define AUTO_HOOK_OFF	0
#define AUTO_HOOK_ON	1

/* to control debugger RPC mode */
#define RPC_M_NORMAL	0
#define RPC_M_NOWAIT	1

typedef struct {
	int auto_hook;
	int rpc_mode;
} debugger_opts_t;

static debugger_opts_t g_debugger_opts;

/*
 * set_debugger_opts - Set debugger options from loader.
 */
void set_debugger_opts(const debugger_opts_t *opts)
{
	g_debugger_opts.auto_hook = opts->auto_hook;
	g_debugger_opts.rpc_mode = opts->rpc_mode;
}

/*
 * load_module_from_kernel - Load IOP module from kernel RAM.
 */
static int load_module_from_kernel(u32 hash, u32 arg_len, const char *args)
{
	const ramfile_t *irx_ptr = (ramfile_t*)IRX_ADDR;
	int irxsize = 0, ret;

	DI();
	ee_kmode_enter();

	/*
	 * find module by hash and copy it to user memory
	 */
	while (irx_ptr->hash) {
		if (irx_ptr->hash == hash) {
			irxsize = irx_ptr->size;
			memcpy(g_buf, irx_ptr->addr, irxsize);
			break;
		}
		irx_ptr++;
	}

	ee_kmode_exit();
	EI();

	/* not found */
	if (!irxsize)
		return -1;

	/* load module from buffer */
	SifExecModuleBuffer(g_buf, irxsize, arg_len, args, &ret);
	return ret;
}

/*
 * post_reboot_hook - Will be executed after IOP reboot to load our modules.
 */
static int post_reboot_hook(void)
{
	int ret;

	/* init services */
	GS_BGCOLOUR = 0xff0000;
	SifInitRpc(0);
	SifLoadFileInit();
	SifInitIopHeap();

	GS_BGCOLOUR = 0xffff00;

	/* load our modules from kernel */
	ret = load_module_from_kernel(HASH_PS2DEV9, 0, NULL);
	if (ret < 0)
		while (1) ;
	ret = load_module_from_kernel(HASH_PS2IP, 0, NULL);
	if (ret < 0)
		while (1) ;
	ret = load_module_from_kernel(HASH_PS2SMAP, 0, NULL);
	if (ret < 0)
		while (1) ;
#if 0
	ret = load_module_from_kernel(HASH_NETLOG, 0, NULL);
	if (ret < 0)
		while (1) ;
#endif
	ret = load_module_from_kernel(HASH_DEBUGGER, 0, NULL);
	if (ret < 0)
		while (1) ;
		
	GS_BGCOLOUR = 0xff00ff;
		
	/* Binding debugger module RPC server */
	rpcNTPBreset();	
	rpcNTPBinit();		
		
	GS_BGCOLOUR = 0x0000ff;

	/* deinit services */
	SifExitIopHeap();
	SifLoadFileExit();	
	SifExitRpc();

	/* automatic padRead hooking is done if needed */
	if (g_debugger_opts.auto_hook)
		patch_padRead();
	
	GS_BGCOLOUR = 0x000000;

#ifdef DISABLE_AFTER_IOPRESET
	_fini();
#endif

	return 1;
}

/*
 * NewSifSetReg - Replacement function for SifSetReg().
 */
void NewSifSetReg(u32 regnum, int regval)
{
	/* call original SifSetReg() */
	OrigSifSetReg(regnum, regval);

	/* catch IOP reboot */
	if (regnum == SIF_REG_SMFLAG && regval == 0x10000) {
		debugger_ready = 0;
		/* by setting set_reg_hook to 4 here, it will reach 0
		 * at the last sceSifSetReg call in sceSifResetIop
		 */  
		set_reg_hook = 4;
	}

	if (set_reg_hook) {
		set_reg_hook--;
		/* check if reboot is done */
		if (!set_reg_hook && regnum == 0x80000000 && !regval) {
			/* we filter the 1st IOP reboot done by LoadExecPS2 or its replacement function */
			if (iop_reboot_count) { 
				/* IOP sync, needed since at this point it haven't yet been done by the game */
				while (!(SifGetReg(SIF_REG_SMFLAG) & 0x40000))
					;
				/* load our modules */
				post_reboot_hook();
				debugger_ready = 1;
			}
			iop_reboot_count++;
			_iop_reboot_count++;			
		}
	}
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
static int sendDump(int dump_type, u32 dump_start, u32 dump_end)
{
	int r, len, sndSize, dumpSize, dpos, rpos;	
									
	if (dump_type == IOP_DUMP) {
		dump_start |= 0xbc000000;
		dump_end   |= 0xbc000000;
	}
	
	len = dump_end - dump_start;
	
	/* reducing dump size to fit in buffer */
	if (len > BUFSIZE)
		dumpSize = BUFSIZE;
	else		
		dumpSize = len;
	
	dpos = 0;	
	while (dpos < len) {
		
		/* dump mem part */							
		read_mem((void *)(dump_start + dpos), dumpSize, g_buf);
			
		/* reducing send size for rpc if needed */
		if (dumpSize > 8192)
			sndSize = 8192;
		else		
			sndSize = dumpSize;
		
		/* sending dump part datas */		
		rpos = 0;
		while (rpos < dumpSize) {
			rpcNTPBsendData(PRINT_DUMP + dump_type, &g_buf[rpos], sndSize, g_debugger_opts.rpc_mode);
			rpcNTPBSync(0, NULL, &r);										
			rpos += sndSize;
			if ((dumpSize - rpos) < 8192)
				sndSize = dumpSize - rpos;				
		}
		
		dpos += dumpSize;	
		if ((len - dpos) < BUFSIZE)
			dumpSize = len - dpos;				
	}

	/* send end of reply message */
	rpcNTPBEndReply(g_debugger_opts.rpc_mode);
	rpcNTPBSync(0, NULL, &r);										
		
	return len;
}

/*
 * execRemoteCmd: this function retrieve a Request sent by the client and fill it
 */
static int execRemoteCmd(void)
{
	u16 remote_cmd;
	int size;
	int ret;
	u8 cmd_buf[64]; 
	
	/* get the remote command by RPC */
	rpcNTPBgetRemoteCmd(&remote_cmd, cmd_buf, &size, g_debugger_opts.rpc_mode);
	rpcNTPBSync(0, NULL, &ret);
		
	if (remote_cmd != REMOTE_CMD_NONE) {
		/* handle Dump requests */
		if (remote_cmd == REMOTE_CMD_DUMPEE) {
			sendDump(EE_DUMP, *((u32 *)&cmd_buf[0]), *((u32 *)&cmd_buf[4]));
		}
		else if (remote_cmd == REMOTE_CMD_DUMPIOP) {
			sendDump(IOP_DUMP, *((u32 *)&cmd_buf[0]), *((u32 *)&cmd_buf[4]));
		}
		else if (remote_cmd == REMOTE_CMD_DUMPKERNEL) {
			sendDump(KERNEL_DUMP, *((u32 *)&cmd_buf[0]), *((u32 *)&cmd_buf[4]));
		}
		else if (remote_cmd == REMOTE_CMD_DUMPSCRATCHPAD) {
			sendDump(SCRATCHPAD_DUMP, *((u32 *)&cmd_buf[0]), *((u32 *)&cmd_buf[4]));
		}
		/* handle Halt request */
		else if (remote_cmd == REMOTE_CMD_HALT) {
			rpcNTPBEndReply(g_debugger_opts.rpc_mode);
			rpcNTPBSync(0, NULL, &ret);													
			if (!haltState) {
				haltState = 1;
				while (haltState)
					execRemoteCmd();
			}								
		}
		/* handle Resume request */
		else if (remote_cmd == REMOTE_CMD_RESUME) {
			rpcNTPBEndReply(g_debugger_opts.rpc_mode);
			rpcNTPBSync(0, NULL, &ret);	
			if (haltState) {			
				haltState = 0; 
			}			
		}
		/* handle raw mem patches adding */		
		else if (remote_cmd == REMOTE_CMD_ADDMEMPATCHES) {
			rpcNTPBEndReply(g_debugger_opts.rpc_mode);
			rpcNTPBSync(0, NULL, &ret);	
			/*
			 * TODO ...
			 */ 
		}
		/* handle raw mem patches clearing */
		else if (remote_cmd == REMOTE_CMD_CLEARMEMPATCHES) {
			rpcNTPBEndReply(g_debugger_opts.rpc_mode);
			rpcNTPBSync(0, NULL, &ret);	
			/*
			 * TODO ...
			 */ 
		}
		/* handle codes adding */
		else if (remote_cmd == REMOTE_CMD_ADDRAWCODES) {
			rpcNTPBEndReply(g_debugger_opts.rpc_mode);
			rpcNTPBSync(0, NULL, &ret);	
			/*
			 * TODO ...
			 */ 
		}
		/* handle codes clearing */
		else if (remote_cmd == REMOTE_CMD_CLEARRAWCODES) {
			rpcNTPBEndReply(g_debugger_opts.rpc_mode);
			rpcNTPBSync(0, NULL, &ret);	
			/*
			 * TODO ...
			 */ 			
		}
	}
	
	return 1;
}

/*
 * _init - Automatically invoked on ERL load.
 */
int _init(void)
{
	/* Set default debugger options */
	g_debugger_opts.auto_hook = AUTO_HOOK_OFF;
	g_debugger_opts.rpc_mode = RPC_M_NOWAIT;	
	
	/* Hook syscalls */
	OldSifSetReg = GetSyscall(__NR_SifSetReg);
	SetSyscall(__NR_SifSetReg, HookSifSetReg);
	SetSyscall(__NR_OrigSifSetReg, OldSifSetReg);

	return 0;
}

/*
 * _fini - Automatically invoked on ERL unload.
 */
int _fini(void)
{
	/* Unhook syscalls */
	SetSyscall(__NR_SifSetReg, OldSifSetReg);
	SetSyscall(__NR_OrigSifSetReg, 0);

	return 0;
}

/*
 * debugger_loop - Constantly called by the cheat engine.
 */
int debugger_loop(void)
{
	/* check/execute remote command */
	if (debugger_ready)
		execRemoteCmd();
	
	return 0;
}
