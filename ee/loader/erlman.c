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
#include "configman.h"
#include "erlman.h"
#include "dbgprintf.h"

#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))

typedef struct {
	char name[20];
	u8 *start;
	struct erl_record_t *erl;
} erl_file_t;

typedef struct {
	int auto_hook;
	int rpc_mode;
} debugger_opts_t;

enum {
//	ERL_FILE_ENGINE = 0,
	ERL_FILE_LIBKERNEL,
#ifdef _DEBUG
	ERL_FILE_LIBC,
	ERL_FILE_LIBDEBUG,
	ERL_FILE_LIBPATCHES,
#endif
	ERL_FILE_DEBUGGER,
	ERL_FILE_ELFLDR,

	ERL_FILE_NUM /* number of files */
};

/* Statically linked ERL files */
extern u8  _libkernel_erl_start[];
#ifdef _DEBUG
extern u8  _libc_erl_start[];
extern u8  _libdebug_erl_start[];
extern u8  _libpatches_erl_start[];
#endif
extern u8  _debugger_erl_start[];
extern u8  _elfldr_erl_start[];

static erl_file_t _erl_files[ERL_FILE_NUM] = {
#if 0
	{
		.name = "engine.erl",
		.start = _engine_erl_start,
	},
#endif
	{
		.name = "libkernel.erl",
		.start = _libkernel_erl_start,
	},
#ifdef _DEBUG
	{
		.name = "libc.erl",
		.start = _libc_erl_start,
	},
	{
		.name = "libdebug.erl",
		.start = _libdebug_erl_start,
	},
	{
		.name = "libpatches.erl",
		.start = _libpatches_erl_start,
	},
#endif
	{
		.name = "debugger.erl",
		.start = _debugger_erl_start,
	},
	{
		.name = "elfldr.erl",
		.start = _elfldr_erl_start,
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

int install_erls(const config_t *config, engine_t *engine)
{
	erl_file_t *file;
	struct symbol_t *sym;
	u32 addr;

#define GET_SYMBOL(var, name, file) \
	sym = erl_find_local_symbol(name, file->erl); \
	if (sym == NULL) { \
		D_PRINTF("%s: could not find symbol '%s'\n", __FUNCTION__, name); \
		return -1; \
	} \
	var = (typeof(var))sym->address

	/*
	 * install SDK libraries
	 */
	if (config_get_bool(config, SET_SDKLIBS_INSTALL)) {
		addr = config_get_u32(config, SET_SDKLIBS_ADDR);
		file = &_erl_files[ERL_FILE_LIBKERNEL];
		if (__install_erl(file, addr) < 0)
			return -1;
#ifdef _DEBUG
		addr += ALIGN(file->erl->fullsize, 64);
		file = &_erl_files[ERL_FILE_LIBC];
		if (__install_erl(file, addr) < 0)
			return -1;

		addr += ALIGN(file->erl->fullsize, 64);
		file = &_erl_files[ERL_FILE_LIBDEBUG];
		if (__install_erl(file, addr) < 0)
			return -1;

		addr += ALIGN(file->erl->fullsize, 64);
		file = &_erl_files[ERL_FILE_LIBPATCHES];
		if (__install_erl(file, addr) < 0)
			return -1;
#endif
	}

	/*
	 * install elfldr
	 */
	if (config_get_bool(config, SET_ELFLDR_INSTALL)) {
		addr = config_get_u32(config, SET_ELFLDR_ADDR);
		file = &_erl_files[ERL_FILE_ELFLDR];

		if (__install_erl(file, addr) < 0)
			return -1;
	}

	/*
	 * install debugger
	 */
	if (config_get_bool(config, SET_DEBUGGER_INSTALL)) {
		addr = config_get_u32(config, SET_DEBUGGER_ADDR);
		file = &_erl_files[ERL_FILE_DEBUGGER];

		if (__install_erl(file, addr) < 0)
			return -1;

		/* add debugger_loop() callback to engine */
		int (*debugger_loop)(void) = NULL;
		GET_SYMBOL(debugger_loop, "debugger_loop", file);
		engine->callbacks[0] = (u32)debugger_loop;

		/* set debugger options */
		void (*set_debugger_opts)(debugger_opts_t *opts) = NULL;
		debugger_opts_t opts;
		GET_SYMBOL(set_debugger_opts, "set_debugger_opts", file);
		opts.auto_hook = config_get_bool(config, SET_DEBUGGER_AUTO_HOOK);
		opts.rpc_mode = config_get_int(config, SET_DEBUGGER_RPC_MODE);
		set_debugger_opts(&opts);
	}

	return 0;
}

void uninstall_erls(void)
{
	erl_file_t *file;
	int i;

	for (i = 0; i < ERL_FILE_NUM; i++) {
		file = &_erl_files[i];
		if (file->erl != NULL)
			__uninstall_erl(file);
	}
}
