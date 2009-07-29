/*
 * engineman.h - Cheat engine manager
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

#ifndef _ENGINEMAN_H_
#define _ENGINEMAN_H_

#include <tamtypes.h>
#include <erl.h>

/**
 * syscall_hook_t - syscall hook
 * @syscall: syscall number
 * @vector: syscall vector
 * @oldvector: original vector
 */
typedef struct {
	s32	syscall;
	void	*vector;
	void	*oldvector;
} syscall_hook_t;

/**
 * engine_ctx_t - cheat engine context
 * @erl: ERL record
 * @info: engine info
 * @id: engine ID
 * @syscall_hooks: syscall hooks
 * @maxhooks: max number of allowed hooks
 * @numhooks: number of hooks in hook list
 * @hooklist: hook list
 * @maxcodes: max number of allowed codes
 * @numcodes: number of codes in code list
 * @codelist: code list
 * @maxcallbacks: max number of callbacks
 * @callbacks: callback list
 */
typedef struct {
	struct erl_record_t	*erl;
	u32			*info;
	u32			*id;
	syscall_hook_t		*syscall_hooks;
	u32			*maxhooks;
	u32			*numhooks;
	u32			*hooklist;
	u32			*maxcodes;
	u32			*numcodes;
	u32			*codelist;
	u32			*maxcallbacks;
	u32			*callbacks;
} engine_ctx_t;

int engine_install_from_file(const char *filename, u32 addr, engine_ctx_t *ctx);
int engine_install_from_mem(u8 *mem, u32 addr, engine_ctx_t *ctx);
void engine_uninstall(engine_ctx_t *ctx);
int engine_add_hook(engine_ctx_t *ctx, u32 addr, u32 val);
int engine_add_code(engine_ctx_t *ctx, u32 addr, u32 val);
void engine_clear_hooks(engine_ctx_t *ctx);
void engine_clear_codes(engine_ctx_t *ctx);

#endif /*_ENGINEMAN_H_*/
