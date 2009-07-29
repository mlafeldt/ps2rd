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
 * @ctx: ptr to engine context
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
static int __engine_install(const char *filename, u8 *mem, u32 addr, engine_ctx_t *ctx)
{
	struct symbol_t *sym;

	memset(ctx, 0, sizeof(engine_ctx_t));

	D_PRINTF("* Installing engine...\n");
	D_PRINTF("ERL addr = %08x\n", addr);

	/* Relocate engine */
	if (filename != NULL)
		ctx->erl = load_erl_from_file_to_addr(filename, addr, 0, NULL);
	else if (mem != NULL)
		ctx->erl = load_erl_from_mem_to_addr(mem, addr, 0, NULL);

	if (ctx->erl == NULL) {
		D_PRINTF("%s: ERL load error\n", __FUNCTION__);
		return -1;
	}

	D_PRINTF("ERL size = %u\n", ctx->erl->fullsize);

	/* Always clear this ERL when unloading */
	ctx->erl->flags |= ERL_FLAG_CLEAR;

	/* Populate engine context */
#define GETSYM(x, s) \
	sym = erl_find_local_symbol(s, ctx->erl); \
	if (sym == NULL) { \
		D_PRINTF("%s: could not find symbol '%s'\n", __FUNCTION__, s); \
		engine_uninstall(ctx); \
		return -2; \
	} \
	x = (typeof(x))sym->address; \
	D_PRINTF("%08x %s\n", sym->address, s)

	GETSYM(ctx->info, "engine_info");
	GETSYM(ctx->id, "engine_id");
	*ctx->id = g_engine_id++;
	GETSYM(ctx->syscall_hooks, "syscall_hooks");
	GETSYM(ctx->maxhooks, "maxhooks");
	GETSYM(ctx->numhooks, "numhooks");
	GETSYM(ctx->hooklist, "hooklist");
	GETSYM(ctx->maxcodes, "maxcodes");
	GETSYM(ctx->numcodes, "numcodes");
	GETSYM(ctx->codelist, "codelist");
	GETSYM(ctx->maxcallbacks, "maxcallbacks");
	GETSYM(ctx->callbacks, "callbacks");

#ifndef _NO_HOOK
	/* Hook syscalls */
	D_PRINTF("Hooking syscalls...\n");
	if (ctx->syscall_hooks != NULL) {
		syscall_hook_t *h = ctx->syscall_hooks;
		while (h->syscall) {
			void *v = hook_syscall(h->syscall, h->vector);
		        if (v == NULL) {
                		D_PRINTF("%s: syscall hook error\n", __FUNCTION__);
				engine_uninstall(ctx);
		                return -3;
		        }
			h->oldvector = v;
			h++;
		}
	}
#else
	D_PRINTF("No syscalls hooked.\n");
#endif
	D_PRINTF("Engine installed (info: %08x id: %i).\n",
		*ctx->info, *ctx->id);

	return 0;
}

/**
 * engine_install_from_file - Installs a cheat engine from file.
 * @filename: name of engine ERL file
 * @addr: load address
 * @ctx: ptr to engine context
 * @return: 0: success, <0: error
 */
int engine_install_from_file(const char *filename, u32 addr, engine_ctx_t *ctx)
{
	return __engine_install(filename, NULL, addr, ctx);
}

/**
 * engine_install_from_mem - Installs a cheat engine from memory.
 * @mem: ptr to ERL in memory
 * @addr: load address
 * @ctx: ptr to engine context
 * @return: 0: success, <0: error
 */
int engine_install_from_mem(u8 *mem, u32 addr, engine_ctx_t *ctx)
{
	return __engine_install(NULL, mem, addr, ctx);
}

/**
 * engine_uninstall - Uninstall a cheat engine.
 * @ctx: ptr to engine context
 */
void engine_uninstall(engine_ctx_t *ctx)
{
	D_PRINTF("* Uninstalling engine (%i)...\n", *ctx->id);

#ifndef _NO_HOOK
	/* Unhook syscalls */
	D_PRINTF("Unhooking syscalls...\n");
	if (ctx->syscall_hooks != NULL) {
		syscall_hook_t *h = ctx->syscall_hooks;
		while (h->syscall) {
			if (h->oldvector != NULL)
				SetSyscall(h->syscall, h->oldvector);
			h++;
		}
	}
#endif
	/* Unload ERL */
	D_PRINTF("Unloading ERL...\n");
	if (ctx->erl != NULL) {
		if (!unload_erl(ctx->erl)) {
			D_PRINTF("%s: ERL unload error\n", __FUNCTION__);
			return -2;
		}
	}

	/* Empty context */
	memset(ctx, 0, sizeof(engine_ctx_t));

	D_PRINTF("Uninstall complete.\n");
}

/**
 * engine_add_hook - Add a hook to an engine's hook list.
 * @ctx: ptr to engine context
 * @addr: hook address
 * @val: hook value
 * @return: 0: success, -1: max hooks reached
 */
int engine_add_hook(engine_ctx_t *ctx, u32 addr, u32 val)
{
	if (*ctx->numhooks >= *ctx->maxhooks)
		return -1;

	ctx->hooklist[*ctx->numhooks * 2] = addr & 0x01FFFFFC;
	ctx->hooklist[*ctx->numhooks * 2 + 1] = val;
	(*ctx->numhooks)++;

	return 0;
}

/**
 * engine_add_code - Add a code to an engine's code list.
 * @ctx: ptr to engine context
 * @addr: code address
 * @val: code value
 * @return: 0: success, -1: max codes reached
 */
int engine_add_code(engine_ctx_t *ctx, u32 addr, u32 val)
{
	if (*ctx->numcodes >= *ctx->maxcodes)
		return -1;

	ctx->codelist[*ctx->numcodes * 2] = addr;
	ctx->codelist[*ctx->numcodes * 2 + 1] = val;
	(*ctx->numcodes)++;

	return 0;
}

/**
 * engine_clear_hooks - Clear all hooks in an engine's hook list.
 * @ctx: ptr to engine context
 */
void engine_clear_hooks(engine_ctx_t *ctx)
{
	int i;

	for (i = 0; i < *ctx->numhooks; i++)
		_sd(0, (u32)&ctx->hooklist[i * 2]);
}

/**
 * engine_clear_codes - Clear all codes in an engine's code list.
 * @ctx: ptr to engine context
 */
void engine_clear_codes(engine_ctx_t *ctx)
{
	int i;

	for (i = 0; i < *ctx->numcodes; i++)
		_sd(0, (u32)&ctx->codelist[i * 2]);
}
