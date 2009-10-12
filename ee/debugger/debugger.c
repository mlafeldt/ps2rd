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
#include <iopcontrol.h>
#include <loadfile.h>
#include <sifdma.h>
#include <sifrpc.h>
#include <string.h>
#include <syscallnr.h>
#include <io_common.h>
#include <fileio.h>
#include <sbv_patches.h>
#include <debug.h>

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
	"libpatches",
	NULL
};
#endif

/* loadfile.c function */
extern int SifLoadModuleAsync(const char *path, int arg_len, const char *args);

/* padread_hooks.c functions */
extern void clear_autohook_tab(void);
extern int patch_padRead(void);

/* loadmodule_hooks.c functions */
extern int patch_loadModule(void);

/* debugger_rpc.c functions */
extern int rpcNTPBinit(void);
extern int rpcNTPBreset(void);
extern int rpcNTPBgetRemoteCmd(u16 *cmd, u8 *buf, int *size, int rpc_mode);
extern int rpcNTPBsendData(u16 cmd, u8 *buf, int size, int rpc_mode);
extern int rpcNTPBEndReply(int rpc_mode);
extern int rpcNTPBSync(int mode, int *cmd, int *result);

/* do not link to strcpy() from libc! */
#define __strcpy(dest, src) \
	strncpy(dest, src, strlen(src))

/* GS macro */
#define GS_BGCOLOUR	*((vu32*)0x120000e0)

/* reset packet structs */
typedef struct {
	u32 psize:8;
	u32 dsize:24;
	u32 daddr;
	u32 fcode;
} _sceSifCmdHdr;

typedef struct {
	_sceSifCmdHdr chdr;
	int size;
	int flag;
	char arg[0x50];
} _sceSifCmdResetData __attribute__((aligned(16)));

/* for syscall hooks */
extern void OrigSifSetReg(u32 register_num, int register_value);
extern void HookSifSetReg(u32 register_num, int register_value);
extern int __NR_OrigSifSetReg;
extern u32 HookSifSetDma(SifDmaTransfer_t *sdd, s32 len);
extern u32 (*OldSifSetDma)(SifDmaTransfer_t *sdd, s32 len);
extern int (*OldSifSetReg)(u32 register_num, int register_value);

/* SifSetReg syscall hook counter */
static int set_reg_hook = 0;

/* SifSetDma syscall hook disable flag */
static int dma_hook_disabled = 0;

/* debugger ready state, allow it to know when to start RPC transfers */
static int debugger_ready = 0;

/* libkernel reboot counter */
extern int _iop_reboot_count;

/* buffer for modules load and memory reads */
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
#define HASH_MEMDISK	0x03c3b0eb
#define HASH_EESYNC	0x06bcb043

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
#define SCRATCHPAD_DUMP			4

/* to control Halted/Resumed game state */
static int haltState = 0;

/* to control automatic hooking */
#define AUTO_HOOK_OFF	0
#define AUTO_HOOK_ON	1

/* to control _SifLoadModule patch */
#define PATCH_LM_OFF	0
#define PATCH_LM_ON	1

/* to control debugger RPC mode */
#define RPC_M_NORMAL	0
#define RPC_M_NOWAIT	1

/* to control debugger modules reloading */
#define LOAD_MODULES_OFF	0
#define LOAD_MODULES_ON		1

struct _ip_config {
	char ip[16];
	char netmask[16];
	char gateway[16];
};

typedef struct {
	int auto_hook;
	int patch_loadmodule;
	int unhook_iop_reset;
	int rpc_mode;
	int load_modules;
	struct _ip_config ipconfig;
} debugger_opts_t;

static debugger_opts_t g_debugger_opts;

/* struct for romdir fs */
typedef struct {
	char 	fileName[10];
	u16 	extinfo_size;
	int 	fileSize;
} romdir_t;

/* for IP config */
#define IPCONFIG_MAX_LEN	64
static char g_ipconfig[IPCONFIG_MAX_LEN] __attribute__((aligned(64)));
static int g_ipconfig_len;

/*
 * set_debugger_opts - Set debugger options from loader.
 */
void set_debugger_opts(const debugger_opts_t *opts)
{
	g_debugger_opts.auto_hook = opts->auto_hook;
	g_debugger_opts.patch_loadmodule = opts->patch_loadmodule;
	g_debugger_opts.unhook_iop_reset = opts->unhook_iop_reset;
	g_debugger_opts.rpc_mode = opts->rpc_mode;
	g_debugger_opts.load_modules = opts->load_modules;

	__strcpy(g_debugger_opts.ipconfig.ip, opts->ipconfig.ip);
	__strcpy(g_debugger_opts.ipconfig.netmask, opts->ipconfig.netmask);
	__strcpy(g_debugger_opts.ipconfig.gateway, opts->ipconfig.gateway);
}

/*
 * get_ipconfig - get & build ip config to pass to smap driver
 */
void get_ipconfig(void)
{
	memset(g_ipconfig, 0, IPCONFIG_MAX_LEN);
	g_ipconfig_len = 0;

	/* add ip to g_ipconfig buf */
	strncpy(&g_ipconfig[g_ipconfig_len], g_debugger_opts.ipconfig.ip, 15);
	g_ipconfig_len += strlen(g_debugger_opts.ipconfig.ip) + 1;

	/* add netmask to g_ipconfig buf */
	strncpy(&g_ipconfig[g_ipconfig_len], g_debugger_opts.ipconfig.netmask, 15);
	g_ipconfig_len += strlen(g_debugger_opts.ipconfig.netmask) + 1;

	/* add gateway to g_ipconfig buf */
	strncpy(&g_ipconfig[g_ipconfig_len], g_debugger_opts.ipconfig.gateway, 15);
	g_ipconfig_len += strlen(g_debugger_opts.ipconfig.gateway) + 1;
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
 * get_module_from_kernel - get IOP module from kernel RAM to buffer on user mem.
 */
static int get_module_from_kernel(u32 hash, void *buf)
{
	const ramfile_t *irx_ptr = (ramfile_t*)IRX_ADDR;
	int irxsize = 0;

	DI();
	ee_kmode_enter();

	/*
	 * find module by hash and copy it to user memory
	 */
	while (irx_ptr->hash) {
		if (irx_ptr->hash == hash) {
			irxsize = irx_ptr->size;
			memcpy(buf, irx_ptr->addr, irxsize);
			break;
		}
		irx_ptr++;
	}

	ee_kmode_exit();
	EI();

	/* not found */
	if (!irxsize)
		return -1;

	return irxsize;
}

/*
 * Get_Mod_Offset - return a module offset in IOPRP img
 */
int Get_Mod_Offset(romdir_t *romdir_fs, const char *modname)
{
	int offset = 0;
	int i, modname_len;

	/* scan romdir fs for module */
	while (strlen(romdir_fs->fileName) > 0) {

		modname_len = strlen(modname);
		char *p = (char *)romdir_fs->fileName;

		for (i=0; ((i<modname_len) && (*p != 0)) ; i++) {
			if (modname[i] != *p)
				break;
			p++;
		}

		if (i == modname_len)
			return offset;

		/* arrange irx size to next 16 bytes multiple to get offset of the module */
		if ((romdir_fs->fileSize % 0x10)==0)
			offset += romdir_fs->fileSize;
		else
			offset += (romdir_fs->fileSize + 0x10) & 0xfffffff0;

		romdir_fs++;
	}

	return -1;
}

/*
 * MySifRebootIop - IOP reset function
 */
int MySifRebootIop(char *ioprp_path)
{
 	void   *ioprp_dest;
	int     ret, fd, ioprp_size, rd_size, rpos, gbuf_offset, rp_size;
	int 	ioprp_offset = 0, eesync_size = 0, eesync_offset = 0;
	u8 	   *eesync_ptr = NULL;
	u32  	qid;
	SifDmaTransfer_t dmat;

	GS_BGCOLOUR = 0xff0000; /* blue */

	/* Reset IOP */
	SifInitRpc(0);
	dma_hook_disabled = 1;
	while (!SifIopReset(NULL, 0)) {;}
	while (!SifIopSync()){;}
	dma_hook_disabled = 0;

	/* Init services & apply SBV patches */
	SifInitRpc(0);
	SifLoadFileInit();
	SifInitIopHeap();
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();

	/* read IOPRP img and send it on IOP mem */
	GS_BGCOLOUR = 0x00ffff; /* yellow */
	fioInit();
	fd = fioOpen(ioprp_path, O_RDONLY);
	if (fd < 0) {
		GS_BGCOLOUR = 0x0000ff; /* red */
		while (1){;}
	}

	/* get IOPRP img size */
	ioprp_size = fioLseek(fd, 0, SEEK_END);
	fioLseek(fd, 0, SEEK_SET);

	/* alloc mem on IOP for the IOPRP img */
	SifInitIopHeap();
	ioprp_dest = SifAllocIopHeap(ioprp_size);
	if (!ioprp_dest) {
		GS_BGCOLOUR = 0x0000ff; /* red */
		while (1){;}
	}

	/* read/send IOPRP to IOP mem using g_buf */
	rpos = 0;
	while (rpos < ioprp_size) {
		if ((ioprp_size - rpos) > BUFSIZE)
			rd_size = BUFSIZE;
		else
			rd_size = ioprp_size - rpos;
		ret = fioRead(fd, g_buf, rd_size);
		if (ret != rd_size) {
			GS_BGCOLOUR = 0x0000ff; /* red */
			while (1){;}
		}

		/* if it's the 1st read then locate EESYNC module
		 * offset in IOPRP and its replacement in kernel RAM
		 */
		if (rpos == 0) {
			ioprp_offset = Get_Mod_Offset((romdir_t *)g_buf, "EESYNC");

			const ramfile_t *irx_ptr = (ramfile_t*)IRX_ADDR;

			DI();
			ee_kmode_enter();

			/* find module by hash */
			while (irx_ptr->hash) {
				if (irx_ptr->hash == HASH_EESYNC) {
					/* Get EESYNC replacement module address & size */
					eesync_ptr = irx_ptr->addr;
					eesync_size = irx_ptr->size;
					break;
				}
				irx_ptr++;
			}

			ee_kmode_exit();
			EI();
		}

		/* replace EESYNC module before to send on IOP mem,
		 * taking care if it's splitted along g_buf
		 */
		if (((rpos + rd_size) >= ioprp_offset) && (eesync_size > 0)) {

			gbuf_offset = ioprp_offset - rpos;

			if (gbuf_offset < 0)
				gbuf_offset = 0;

			DI();
			ee_kmode_enter();

			if ((rd_size - gbuf_offset) < eesync_size)
				rp_size = rd_size - gbuf_offset;
			else
				rp_size = eesync_size;

			if (rp_size)
				memcpy(&g_buf[gbuf_offset], &eesync_ptr[eesync_offset], rp_size);

			eesync_offset += rd_size - gbuf_offset;
			eesync_size -= 	rd_size - gbuf_offset;

			ee_kmode_exit();
			EI();

			FlushCache(0);
		}

		/* send g_buf on IOP mem */
		dmat.src = (void *)g_buf;
		dmat.dest = (void *)(ioprp_dest + rpos);
		dmat.size = rd_size;
		dmat.attr = 0;

		qid = SifSetDma(&dmat, 1);
		while (SifDmaStat(qid) >= 0);

		rpos += rd_size;
	}

	fioClose(fd);
	fioExit();

	/* we also want to get memdisk in g_buf without loading it */
	int size_memdisk_irx = get_module_from_kernel(HASH_MEMDISK, g_buf);

	/* Patching memdisk irx */
	u8 *memdisk_drv = (u8 *)g_buf;
	*(u32*)(&memdisk_drv[0x19C]) = (u32)ioprp_dest;
	*(u32*)(&memdisk_drv[0x1A8]) = ioprp_size;
	FlushCache(0);

	/* use MDISK device to reload modules asynchronously, using NOWAIT RPC mode */
	SifExecModuleBuffer(memdisk_drv, size_memdisk_irx, 0, NULL, &ret);
	SifLoadModuleAsync("rom0:UDNL", 7, "MDISK0:");

	GS_BGCOLOUR = 0x00ff00; /* lime */

	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();

	/* as sceSifIopReset does... */
	OrigSifSetReg(SIF_REG_SMFLAG, 0x10000);
	OrigSifSetReg(SIF_REG_SMFLAG, 0x20000);
	OrigSifSetReg(0x80000002, 0);
	OrigSifSetReg(0x80000000, 0);

	/* sync IOP */
	while (!SifIopSync()) {;}

	GS_BGCOLOUR = 0xffff00; /* cyan */

	SifInitRpc(0);
	SifLoadFileInit();
	SifInitIopHeap();
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();

	if (g_debugger_opts.load_modules == LOAD_MODULES_ON) {
		/* load our modules from kernel */
		ret = load_module_from_kernel(HASH_PS2DEV9, 0, NULL);
		if (ret < 0)
			while (1) ;
		ret = load_module_from_kernel(HASH_PS2IP, 0, NULL);
		if (ret < 0)
			while (1) ;
		get_ipconfig();
		ret = load_module_from_kernel(HASH_PS2SMAP, g_ipconfig_len, g_ipconfig);
		if (ret < 0)
			while (1) ;
		ret = load_module_from_kernel(HASH_DEBUGGER, 0, NULL);
		if (ret < 0)
		while (1) ;

		GS_BGCOLOUR = 0xff00ff; /* magenta */

		/* Binding debugger module RPC server */
		rpcNTPBreset();
		rpcNTPBinit();
	}

	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();

	/* automatic padRead hooking is done if needed */
	if (g_debugger_opts.auto_hook == AUTO_HOOK_ON)
		patch_padRead();

	/* automatic _SifLoadModule hooking is done if needed */
	if (g_debugger_opts.patch_loadmodule == PATCH_LM_ON)
		patch_loadModule();

	GS_BGCOLOUR = 0x000000; /* black */

	/* set number of SifSetReg hooks to skip */
	set_reg_hook = 4;

	return 1;
}

/*
 * NewSifSetDma - Replacement function for SifSetDma().
 */
u32 NewSifSetDma(SifDmaTransfer_t *sdd, s32 len)
{
	int i, j;
 	char ioprp_path[0x50];
 	int ioprp_img = 0;
	_sceSifCmdResetData *reset_pkt = (_sceSifCmdResetData*)sdd->src;

	/* check for reboot with IOPRP from cdrom */
	char *ioprp_pattern = "rom0:UDNL cdrom";
	for (i=0; i<0x50; i++) {
		for (j=0; j<15; j++) {
			if (reset_pkt->arg[i+j] != ioprp_pattern[j])
				break;
		}
		if (j == 15) {
			/* get IOPRP img file path */
			__strcpy(ioprp_path, &reset_pkt->arg[i+10]);
			ioprp_img = 1;
			break;
		}
	}

	if ((!dma_hook_disabled) && (ioprp_img)) {
		/* increment libkernel's IOP reboot counter, this allow
		 * to detect RPC services unbinding
	 	 */
		_iop_reboot_count++;

		/* it's a reset with cdrom IOPRP so we perform it ourselves */
		MySifRebootIop(ioprp_path);
	}
	else {
		/* not a reset with cdrom IOPRP, so we must send reset packet
		 * with SifSetDma and let the system do IOP reboot
		 */
		DI();
		ee_kmode_enter();
		OldSifSetDma(sdd, len);
		ee_kmode_exit();
		EI();
	}

	return 1;
}

/*
 * NewSifSetReg - Replacement function for SifSetReg().
 */
void NewSifSetReg(u32 regnum, int regval)
{
	/* Skip all 4 SifSetReg made after SifSetDma in sceSifResetIop */
	if (set_reg_hook) {
		set_reg_hook--;

		if (set_reg_hook == 0) {
			/* IOP reset hook is complete so debugger is ready to dump */
			debugger_ready = 1;

			/* IOP reset unhook is done if needed */
			if (g_debugger_opts.unhook_iop_reset) {
				SetSyscall(__NR_SifSetDma, OldSifSetDma);
				SetSyscall(__NR_SifSetReg, OldSifSetReg);
				SetSyscall(__NR_OrigSifSetReg, 0);
			}
		}
		return;
	}

	/* Call original SifSetReg */
	OrigSifSetReg(regnum, regval);
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

	if (g_debugger_opts.rpc_mode == -1)
		return 0;

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
	g_debugger_opts.patch_loadmodule = PATCH_LM_OFF;
	g_debugger_opts.unhook_iop_reset = 0;
	g_debugger_opts.rpc_mode = RPC_M_NOWAIT;
	g_debugger_opts.load_modules = LOAD_MODULES_ON;

	/* set default IP settings */
	__strcpy(g_debugger_opts.ipconfig.ip, "192.168.0.10");
	__strcpy(g_debugger_opts.ipconfig.netmask, "255.255.255.0");
	__strcpy(g_debugger_opts.ipconfig.gateway, "192.168.0.1");

	/* clear auto-hook addresses table */
	clear_autohook_tab();

	/* Hook syscalls */
	OldSifSetDma = GetSyscallHandler(__NR_SifSetDma);
	SetSyscall(__NR_SifSetDma, HookSifSetDma);

	OldSifSetReg = GetSyscallHandler(__NR_SifSetReg);
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
	SetSyscall(__NR_SifSetDma, OldSifSetDma);

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
