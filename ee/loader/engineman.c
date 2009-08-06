/*
 * engineman.c - Cheat engine manager
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
#include <string.h>
#include <erl.h>
#include "myutil.h"
#include "engineman.h"
#include "dbgprintf.h"

/* Current engine ID */
static int g_engine_id = 1;

/*
 * engine_install - Installs a cheat engine from file or memory.
 * @filename: name of engine ERL file
 * @mem: ptr to ERL in memory
 * @addr: load address
 * @engine: engine context
 * @return: 0: success, <0: error
 *
 * The setup involves the following:
 *  - relocate engine at specified memory address
 *  - populate engine context
 *  - hook syscalls
 *
 * Note: It should be possible to have multiple engines running concurrently,
 * provided they don't impede each other.
 */
static int __engine_install(const char *filename, u8 *mem, u32 addr, engine_t *engine)
{
	struct symbol_t *sym;

	memset(engine, 0, sizeof(engine_t));

	D_PRINTF("%s: addr=%08x\n", __FUNCTION__, addr);

	/* Relocate engine */
	if (filename != NULL)
		engine->erl = load_erl_from_file_to_addr(filename, addr, 0, NULL);
	else if (mem != NULL)
		engine->erl = load_erl_from_mem_to_addr(mem, addr, 0, NULL);

	if (engine->erl == NULL) {
		D_PRINTF("%s: ERL load error\n", __FUNCTION__);
		return -1;
	}

	FlushCache(0);

	D_PRINTF("%s: size=%u\n", __FUNCTION__, engine->erl->fullsize);

	/* Always clear this ERL when unloading */
	engine->erl->flags |= ERL_FLAG_CLEAR;

	/* Populate engine context */
#define GETSYM(x, s) \
	sym = erl_find_local_symbol(s, engine->erl); \
	if (sym == NULL) { \
		D_PRINTF("%s: could not find symbol '%s'\n", __FUNCTION__, s); \
		engine_uninstall(engine); \
		return -2; \
	} \
	x = (typeof(x))sym->address; \
	D_PRINTF("%08x %s\n", sym->address, s)

	GETSYM(engine->info, "engine_info");
	GETSYM(engine->id, "engine_id");
	*engine->id = g_engine_id++;
	GETSYM(engine->syscall_hooks, "syscall_hooks");
	GETSYM(engine->maxhooks, "maxhooks");
	GETSYM(engine->numhooks, "numhooks");
	GETSYM(engine->hooklist, "hooklist");
	GETSYM(engine->maxcodes, "maxcodes");
	GETSYM(engine->numcodes, "numcodes");
	GETSYM(engine->codelist, "codelist");
	GETSYM(engine->maxcallbacks, "maxcallbacks");
	GETSYM(engine->callbacks, "callbacks");

#ifndef _NO_HOOK
	/* Hook syscalls */
	D_PRINTF("%s: hooking syscalls...\n", __FUNCTION__);
	syscall_hook_t *h = engine->syscall_hooks;
	while (h->syscall) {
		void *v = hook_syscall(h->syscall, h->vector);
		if (v == NULL) {
			D_PRINTF("%s: syscall hook error\n", __FUNCTION__);
			engine_uninstall(engine);
			return -3;
		}
		h->oldvector = v;
		h++;
	}
#else
	D_PRINTF("%s: no syscalls hooked.\n", __FUNCTION__);
#endif
	D_PRINTF("%s: install completed (info=%08x id=%i).\n",
		__FUNCTION__, *engine->info, *engine->id);

	return 0;
}

/**
 * engine_install_from_file - Installs a cheat engine from file.
 * @filename: name of engine ERL file
 * @addr: load address
 * @engine: engine context
 * @return: 0: success, <0: error
 */
int engine_install_from_file(const char *filename, u32 addr, engine_t *engine)
{
	return __engine_install(filename, NULL, addr, engine);
}

/**
 * engine_install_from_mem - Installs a cheat engine from memory.
 * @mem: ptr to ERL in memory
 * @addr: load address
 * @engine: engine context
 * @return: 0: success, <0: error
 */
int engine_install_from_mem(u8 *mem, u32 addr, engine_t *engine)
{
	return __engine_install(NULL, mem, addr, engine);
}

/**
 * engine_uninstall - Uninstall a cheat engine.
 * @engine: engine context
 * @return: 0: success, <0: error
 */
int engine_uninstall(engine_t *engine)
{
	D_PRINTF("%s: id=%i\n", __FUNCTION__, *engine->id);

#ifndef _NO_HOOK
	/* Unhook syscalls */
	D_PRINTF("%s: unhooking syscalls...\n", __FUNCTION__);
	syscall_hook_t *h = engine->syscall_hooks;
	while (h->syscall) {
		if (h->oldvector != NULL)
			SetSyscall(h->syscall, h->oldvector);
		h++;
	}
#endif
	/* Unload ERL */
	D_PRINTF("%s: unloading ERL...\n", __FUNCTION__);
	if (engine->erl != NULL) {
		if (!unload_erl(engine->erl)) {
			D_PRINTF("%s: ERL unload error\n", __FUNCTION__);
			return -1;
		}
	}

	/* Empty context */
	memset(engine, 0, sizeof(engine_t));

	D_PRINTF("%s: uninstall completed.\n", __FUNCTION__);

	return 0;
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
