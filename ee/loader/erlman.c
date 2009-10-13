/*
 * erlman.c - ERL file manager
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

/* Statically linked ERL files */
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
	{
		.name = "engine.erl",
		.start = _engine_erl_start,
	},
	{
		.name = "libkernel.erl",
		.start = _libkernel_erl_start,
	},
#if 0
	{
		.name = "libc.erl",
		.start = _libc_erl_start,
	},
	{
		.name = "libdebug.erl",
		.start = _libdebug_erl_start,
	},
#endif
	{
		.name = "libpatches.erl",
		.start = _libpatches_erl_start,
	},
	{
		.name = "debugger.erl",
		.start = _debugger_erl_start,
	},
	{
		.name = "elfldr.erl",
		.start = _elfldr_erl_start,
	},
	{
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
	var = (typeof(var))sym->address

	/*
	 * install SDK libraries
	 */
	if (_config_get_bool(config, SET_SDKLIBS_INSTALL)) {
		addr = _config_get_u32(config, SET_SDKLIBS_ADDR);
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
	if (_config_get_bool(config, SET_ENGINE_INSTALL)) {
		addr = _config_get_u32(config, SET_ENGINE_ADDR);
		file = &_erl_files[ERL_FILE_ENGINE];

		if (__install_erl(file, addr) < 0)
			return -1;

		/* populate engine context */
		GET_SYMBOL(engine->info, "engine_info");
		GET_SYMBOL(engine->maxhooks, "maxhooks");
		GET_SYMBOL(engine->numhooks, "numhooks");
		GET_SYMBOL(engine->hooklist, "hooklist");
		GET_SYMBOL(engine->maxcodes, "maxcodes");
		GET_SYMBOL(engine->numcodes, "numcodes");
		GET_SYMBOL(engine->codelist, "codelist");
		GET_SYMBOL(engine->maxcallbacks, "maxcallbacks");
		GET_SYMBOL(engine->callbacks, "callbacks");
	}

	/*
	 * install videomod
	 */
	if (_config_get_bool(config, SET_VIDEOMOD_INSTALL)) {
		addr = _config_get_u32(config, SET_VIDEOMOD_ADDR);
		file = &_erl_files[ERL_FILE_VIDEOMOD];

		if (__install_erl(file, addr) < 0)
			return -1;

		int *vmode;
		GET_SYMBOL(vmode, "vmode");
		*vmode = _config_get_int(config, SET_VIDEOMOD_VMODE);

		if (_config_get_bool(config, SET_VIDEOMOD_YFIX)) {
			int *ydiff;
			void (*YPosHandler)(void) = NULL;

			GET_SYMBOL(ydiff, "ydiff_lores");
			*ydiff = _config_get_int(config, SET_VIDEOMOD_YDIFF_LORES);
			GET_SYMBOL(ydiff, "ydiff_hires");
			*ydiff = _config_get_int(config, SET_VIDEOMOD_YDIFF_HIRES);

			/* trap writes to GS registers DISPLAY1 and DISPLAY2 */
			GET_SYMBOL(YPosHandler, "YPosHandler");
			install_debug_handler(YPosHandler);
			InitBPC();
			SetDataAddrBP(0x12000080, 0xFFFFFFDF, BPC_DWE | BPC_DUE);
			D_PRINTF("%s: y-fix breakpoint set\n", __FUNCTION__);
		}
	}

	/*
	 * install debugger
	 */
	if (_config_get_bool(config, SET_DEBUGGER_INSTALL)) {
		addr = _config_get_u32(config, SET_DEBUGGER_ADDR);
		file = &_erl_files[ERL_FILE_DEBUGGER];

		if (!_config_get_bool(config, SET_SDKLIBS_INSTALL)) {
			D_PRINTF("%s: dependency error: %s needs SDK libs\n",
				__FUNCTION__, file->name);
			return -1;
		}

		if (__install_erl(file, addr) < 0)
			return -1;

		/* add debugger_loop() callback to engine */
		if (_config_get_bool(config, SET_ENGINE_INSTALL)) {
			int (*debugger_loop)(void) = NULL;
			GET_SYMBOL(debugger_loop, "debugger_loop");
			engine->callbacks[0] = (u32)debugger_loop;
		}

		/* set debugger options */
		void (*set_debugger_opts)(debugger_opts_t *opts) = NULL;
		debugger_opts_t opts;

		GET_SYMBOL(set_debugger_opts, "set_debugger_opts");
		memset(&opts, 0, sizeof(opts));
		opts.auto_hook = _config_get_bool(config, SET_DEBUGGER_AUTO_HOOK);
		opts.patch_loadmodule = _config_get_bool(config, SET_DEBUGGER_PATCH_LOADMODULE);
		opts.unhook_iop_reset = _config_get_bool(config, SET_DEBUGGER_UNHOOK_IOP_RESET);
		opts.rpc_mode = _config_get_int(config, SET_DEBUGGER_RPC_MODE);
		opts.load_modules = _config_get_bool(config, SET_DEBUGGER_LOAD_MODULES);
		strncpy(opts.ip_config.ipaddr, _config_get_string(config, SET_DEBUGGER_IPADDR), 15);
		strncpy(opts.ip_config.netmask, _config_get_string(config, SET_DEBUGGER_NETMASK), 15);
		strncpy(opts.ip_config.gateway, _config_get_string(config, SET_DEBUGGER_GATEWAY), 15);
		set_debugger_opts(&opts);
	}

	/*
	 * install ELF loader
	 */
	if (_config_get_bool(config, SET_ELFLDR_INSTALL)) {
		addr = _config_get_u32(config, SET_ELFLDR_ADDR);
		file = &_erl_files[ERL_FILE_ELFLDR];

		if (!_config_get_bool(config, SET_SDKLIBS_INSTALL)) {
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


/**
 * engine_add_hook - Add a hook to an engine's hook list.
 * @engine: engine context
 * @addr: hook address
 * @val: hook value
 * @return: 0: success, -1: max hooks reached
 */
int engine_add_hook(engine_t *engine, u32 addr, u32 val)
{
	if (*engine->numhooks >= *engine->maxhooks)
		return -1;

	engine->hooklist[*engine->numhooks * 2] = addr & 0x01FFFFFC;
	engine->hooklist[*engine->numhooks * 2 + 1] = val;
	(*engine->numhooks)++;

	return 0;
}

/**
 * engine_add_code - Add a code to an engine's code list.
 * @engine: engine context
 * @addr: code address
 * @val: code value
 * @return: 0: success, -1: max codes reached
 */
int engine_add_code(engine_t *engine, u32 addr, u32 val)
{
	if (*engine->numcodes >= *engine->maxcodes)
		return -1;

	engine->codelist[*engine->numcodes * 2] = addr;
	engine->codelist[*engine->numcodes * 2 + 1] = val;
	(*engine->numcodes)++;

	return 0;
}

/**
 * engine_clear_hooks - Clear all hooks in an engine's hook list.
 * @engine: engine context
 */
void engine_clear_hooks(engine_t *engine)
{
	memset(engine->hooklist, 0, *engine->numhooks * 8);
	*engine->numhooks = 0;
}

/**
 * engine_clear_codes - Clear all codes in an engine's code list.
 * @engine: engine context
 */
void engine_clear_codes(engine_t *engine)
{
	memset(engine->codelist, 0, *engine->numcodes * 8);
	*engine->numcodes = 0;
}
