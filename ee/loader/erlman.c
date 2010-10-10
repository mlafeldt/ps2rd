/*
 * ERL file manager
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
 *
 * $Id$
 */

#include <tamtypes.h>
#include <kernel.h>
#include <erl.h>
#include <hwbp.h>
#include <string.h>
#include "configman.h"
#include "erlman.h"
#include "dbgprintf.h"
#include "myutil.h"

#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))

typedef struct {
	char name[20];
	u8 *start;
	struct erl_record_t *erl;
} erl_file_t;

typedef struct {
	int auto_hook;
	int patch_loadmodule;
	int unhook_iop_reset;
	int rpc_mode;
	int load_modules;
	struct {
		char ipaddr[16];
		char netmask[16];
		char gateway[16];
	} ip_config;
} debugger_opts_t;

enum {
	ERL_FILE_ENGINE = 0,
	ERL_FILE_LIBKERNEL,
#if 0
	ERL_FILE_LIBC,
	ERL_FILE_LIBDEBUG,
#endif
	ERL_FILE_LIBPATCHES,
	ERL_FILE_DEBUGGER,
	ERL_FILE_ELFLDR,
	ERL_FILE_VIDEOMOD,

	ERL_FILE_NUM /* number of files */
};

/* statically linked ERL files */
extern u8  _engine_erl_start[];
extern u8  _libkernel_erl_start[];
#if 0
extern u8  _libc_erl_start[];
extern u8  _libdebug_erl_start[];
#endif
extern u8  _libpatches_erl_start[];
extern u8  _debugger_erl_start[];
extern u8  _elfldr_erl_start[];
extern u8  _videomod_erl_start[];

static erl_file_t _erl_files[ERL_FILE_NUM] = {
	[ERL_FILE_ENGINE] = {
		.name = "engine.erl",
		.start = _engine_erl_start,
	},
	[ERL_FILE_LIBKERNEL] = {
		.name = "libkernel.erl",
		.start = _libkernel_erl_start,
	},
#if 0
	[ERL_FILE_LIBC] = {
		.name = "libc.erl",
		.start = _libc_erl_start,
	},
	[ERL_FILE_LIBDEBUG] = {
		.name = "libdebug.erl",
		.start = _libdebug_erl_start,
	},
#endif
	[ERL_FILE_LIBPATCHES] = {
		.name = "libpatches.erl",
		.start = _libpatches_erl_start,
	},
	[ERL_FILE_DEBUGGER] = {
		.name = "debugger.erl",
		.start = _debugger_erl_start,
	},
	[ERL_FILE_ELFLDR] = {
		.name = "elfldr.erl",
		.start = _elfldr_erl_start,
	},
	[ERL_FILE_VIDEOMOD] = {
		.name = "videomod.erl",
		.start = _videomod_erl_start,
	}
};

static int __install_erl(erl_file_t *file, u32 addr)
{
	D_PRINTF("%s: relocate %s at %08x\n", __FUNCTION__, file->name, addr);

	file->erl = load_erl_from_mem_to_addr(file->start, addr, 0, NULL);
	if (file->erl == NULL) {
		D_PRINTF("%s: %s load error\n", __FUNCTION__, file->name);
		return -1;
	}

	file->erl->flags |= ERL_FLAG_CLEAR;

	FlushCache(0);

	D_PRINTF("%s: size=%u end=%08x\n", __FUNCTION__, file->erl->fullsize,
		addr + file->erl->fullsize);

	return 0;
}

static int __uninstall_erl(erl_file_t *file)
{
	D_PRINTF("%s: uninstall %s from %08x\n", __FUNCTION__, file->name,
		(u32)file->erl->bytes);

	if (!unload_erl(file->erl)) {
		D_PRINTF("%s: %s unload error\n", __FUNCTION__, file->name);
		return -1;
	}

	file->erl = NULL;

	return 0;
}

static struct erl_record_t *__init_load_erl_dummy(char *erl_id)
{
	/* Do nothing */
	return NULL;
}

int install_erls(const config_t *config, engine_t *engine)
{
	erl_file_t *file = NULL;
	struct symbol_t *sym = NULL;
	u32 addr;

	/* replace original load function - we resolve dependencies manually */
	_init_load_erl = __init_load_erl_dummy;

#define GET_SYMBOL(var, name) \
	sym = erl_find_local_symbol(name, file->erl); \
	if (sym == NULL) { \
		D_PRINTF("%s: could not find symbol '%s'\n", __FUNCTION__, name); \
		return -1; \
	} \
	D_PRINTF("%08x %s\n", (u32)sym->address, name); \
	var = (typeof(var))sym->address

	/*
	 * install SDK libraries
	 */
	if (config_get_bool(config, SET_SDKLIBS_INSTALL)) {
		addr = config_get_int(config, SET_SDKLIBS_ADDR);
		file = &_erl_files[ERL_FILE_LIBKERNEL];
		if (__install_erl(file, addr) < 0)
			return -1;
#if 0
		addr += ALIGN(file->erl->fullsize, 64);
		file = &_erl_files[ERL_FILE_LIBC];
		if (__install_erl(file, addr) < 0)
			return -1;

		addr += ALIGN(file->erl->fullsize, 64);
		file = &_erl_files[ERL_FILE_LIBDEBUG];
		if (__install_erl(file, addr) < 0)
			return -1;
#endif
		addr += ALIGN(file->erl->fullsize, 64);
		file = &_erl_files[ERL_FILE_LIBPATCHES];
		if (__install_erl(file, addr) < 0)
			return -1;
	} else {
		/* export syscall functions if libkernel isn't installed */
		erl_add_global_symbol("GetSyscallHandler", (u32)GetSyscallHandler);
		erl_add_global_symbol("SetSyscall", (u32)SetSyscall);
	}

	/*
	 * install cheat engine
	 */
	if (config_get_bool(config, SET_ENGINE_INSTALL)) {
		addr = config_get_int(config, SET_ENGINE_ADDR);
		file = &_erl_files[ERL_FILE_ENGINE];

		if (__install_erl(file, addr) < 0)
			return -1;

		/* populate engine context */
		GET_SYMBOL(engine->get_max_hooks, "get_max_hooks");
		GET_SYMBOL(engine->get_num_hooks, "get_num_hooks");
		GET_SYMBOL(engine->add_hook, "add_hook");
		GET_SYMBOL(engine->clear_hooks, "clear_hooks");
		GET_SYMBOL(engine->get_max_codes, "get_max_codes");
		GET_SYMBOL(engine->set_max_codes, "set_max_codes");
		GET_SYMBOL(engine->get_num_codes, "get_num_codes");
		GET_SYMBOL(engine->add_code, "add_code");
		GET_SYMBOL(engine->clear_codes, "clear_codes");
		GET_SYMBOL(engine->register_callback, "register_callback");
	}

	/*
	 * install videomod
	 */
	if (config_get_bool(config, SET_VIDEOMOD_INSTALL)) {
		void (*set_vmode)(int m) = NULL;
		addr = config_get_int(config, SET_VIDEOMOD_ADDR);
		file = &_erl_files[ERL_FILE_VIDEOMOD];

		if (__install_erl(file, addr) < 0)
			return -1;

		GET_SYMBOL(set_vmode, "set_vmode");
		set_vmode(config_get_int(config, SET_VIDEOMOD_VMODE));

		if (config_get_bool(config, SET_VIDEOMOD_YFIX)) {
			void (*set_ydiff)(int lo, int hi) = NULL;
			void (*YPosHandler)(void) = NULL;

			GET_SYMBOL(set_ydiff, "set_ydiff");
			GET_SYMBOL(YPosHandler, "YPosHandler");

			set_ydiff(config_get_int(config, SET_VIDEOMOD_YDIFF_LORES),
				config_get_int(config, SET_VIDEOMOD_YDIFF_HIRES));

			/* trap writes to GS registers DISPLAY1 and DISPLAY2 */
			install_debug_handler(YPosHandler);
			InitBPC();
			SetDataAddrBP(0x12000080, 0xFFFFFFDF, BPC_DWE | BPC_DUE);
			D_PRINTF("%s: y-fix breakpoint set\n", __FUNCTION__);
		}
	}

	/*
	 * install debugger
	 */
	if (config_get_bool(config, SET_DEBUGGER_INSTALL)) {
		addr = config_get_int(config, SET_DEBUGGER_ADDR);
		file = &_erl_files[ERL_FILE_DEBUGGER];

		if (!config_get_bool(config, SET_SDKLIBS_INSTALL)) {
			D_PRINTF("%s: dependency error: %s needs SDK libs\n",
				__FUNCTION__, file->name);
			return -1;
		}

		if (__install_erl(file, addr) < 0)
			return -1;

		/* add debugger_loop() callback to engine */
		if (config_get_bool(config, SET_ENGINE_INSTALL)) {
			int (*debugger_loop)(void) = NULL;
			GET_SYMBOL(debugger_loop, "debugger_loop");
			engine->register_callback(debugger_loop);
		}

		/* set debugger options */
		void (*set_debugger_opts)(debugger_opts_t *opts) = NULL;
		debugger_opts_t opts;

		GET_SYMBOL(set_debugger_opts, "set_debugger_opts");
		memset(&opts, 0, sizeof(opts));
		opts.auto_hook = config_get_bool(config, SET_DEBUGGER_AUTO_HOOK);
		opts.patch_loadmodule = config_get_bool(config, SET_DEBUGGER_PATCH_LOADMODULE);
		opts.unhook_iop_reset = config_get_bool(config, SET_DEBUGGER_UNHOOK_IOP_RESET);
		opts.rpc_mode = config_get_int(config, SET_DEBUGGER_RPC_MODE);
		opts.load_modules = config_get_bool(config, SET_DEBUGGER_LOAD_MODULES);
		strncpy(opts.ip_config.ipaddr, config_get_string(config, SET_DEBUGGER_IPADDR), 15);
		strncpy(opts.ip_config.netmask, config_get_string(config, SET_DEBUGGER_NETMASK), 15);
		strncpy(opts.ip_config.gateway, config_get_string(config, SET_DEBUGGER_GATEWAY), 15);
		set_debugger_opts(&opts);
	}

	/*
	 * install ELF loader
	 */
	if (config_get_bool(config, SET_ELFLDR_INSTALL)) {
		addr = config_get_int(config, SET_ELFLDR_ADDR);
		file = &_erl_files[ERL_FILE_ELFLDR];

		if (!config_get_bool(config, SET_SDKLIBS_INSTALL)) {
			D_PRINTF("%s: dependency error: %s needs SDK libs\n",
				__FUNCTION__, file->name);
			return -1;
		}

		if (__install_erl(file, addr) < 0)
			return -1;
	}

	return 0;
}

void uninstall_erls(void)
{
	erl_file_t *file;
	int i;

	for (i = 0; i < ERL_FILE_NUM; i++) {
		file = &_erl_files[i];
		if (file->erl != NULL) {
			if (i == ERL_FILE_VIDEOMOD) {
				/* disable y-fix breakpoint */
				InitBPC();
				install_debug_handler(NULL);
			}
			__uninstall_erl(file);
		}
	}
}
