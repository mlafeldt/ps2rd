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
#include "dbgprintf.h"
#include "configman.h"
#include "erlman.h"

typedef struct {
	char name[20];
	u8 *start;
	struct erl_record_t *erl;
} erl_file_t;

enum {
//	ERL_FILE_ENGINE = 0,
	ERL_FILE_LIBKERNEL,
	ERL_FILE_LIBC,
	ERL_FILE_LIBDEBUG,
	ERL_FILE_LIBPATCHES,
	ERL_FILE_DEBUGGER,
	ERL_FILE_ELFLDR,

	ERL_FILE_NUM /* tricky */
};

/* Statically linked ERL files */
extern u8  _libkernel_erl_start[];
extern u8  _libc_erl_start[];
extern u8  _libdebug_erl_start[];
extern u8  _libpatches_erl_start[];
extern u8  _debugger_erl_start[];
extern u8  _elfldr_erl_start[];

static erl_file_t _erl_files[ERL_FILE_NUM] = {
#if 0
	{
		.name = "engine",
		.start = _engine_erl_start,
	},
#endif
	{
		.name = "libkernel",
		.start = _libkernel_erl_start,
	},
	{
		.name = "libc",
		.start = _libc_erl_start,
	},
	{
		.name = "libdebug",
		.start = _libdebug_erl_start,
	},
	{
		.name = "libpatches",
		.start = _libpatches_erl_start,
	},
	{
		.name = "debugger",
		.start = _debugger_erl_start,
	},
	{
		.name = "elfldr",
		.start = _elfldr_erl_start,
	}
};

static int __install_erl(erl_file_t *file, u32 addr)
{
	D_PRINTF("%s: relocate %s.erl at %08x\n", __FUNCTION__, file->name, addr);

	file->erl = load_erl_from_mem_to_addr(file->start, addr, 0, NULL);
	if (file->erl == NULL) {
		D_PRINTF("%s: %s load error\n", __FUNCTION__, file->name);
		return -1;
	}

	FlushCache(0);

	D_PRINTF("%s: size=%u end=%08x\n", __FUNCTION__, file->erl->fullsize,
		addr + file->erl->fullsize);

	return 0;
}

int erl_install_libs(const config_t *config)
{
	erl_file_t *file;
	u32 addr = LIBKERNEL_ADDR; /* TODO: get from config */

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

	addr += ALIGN(file->erl->fullsize, 64);
	file = &_erl_files[ERL_FILE_LIBPATCHES];
	if (__install_erl(file, addr) < 0)
		return -1;
#endif
	return 0;
}

/* LoadExecPS2() replacement function from ELF loader */
void (*MyLoadExecPS2)(const char *filename, int argc, char *argv[]) = NULL;

int erl_install_elfldr(const config_t *config)
{
	erl_file_t *file = &_erl_files[ERL_FILE_ELFLDR];
	struct symbol_t *sym;
	u32 addr = ELFLDR_ADDR; /* TODO: get from config */

	if (__install_erl(file, addr) < 0)
		return -1;

	sym = erl_find_local_symbol("MyLoadExecPS2", file->erl);
	if (sym == NULL) {
		D_PRINTF("%s: could not find symbol MyLoadExecPS2\n",
			__FUNCTION__);
		return -2;
	}

	MyLoadExecPS2 = (void*)sym->address;

	return 0;
}

int erl_install_debugger(const config_t *config, engine_t *engine)
{
	erl_file_t *file = &_erl_files[ERL_FILE_DEBUGGER];
	struct symbol_t *sym;
	u32 addr;

	if (!config_get_bool(config, SET_DEBUGGER_INSTALL))
		return 0;

	addr = config_get_u32(config, SET_DEBUGGER_ADDR);

	if (__install_erl(file, addr) < 0)
		return -1;

	sym = erl_find_local_symbol("debugger_loop", file->erl);
	if (sym == NULL) {
		D_PRINTF("%s: could not find symbol debugger_loop\n",
			__FUNCTION__);
		return -2;
	}

	/* add debugger_loop() callback to engine */
	engine->callbacks[0] = (u32)sym->address;

	return 0;
}
