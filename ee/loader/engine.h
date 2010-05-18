/*
 * engine.h
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
 */

#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <tamtypes.h>
#include <string.h>

/**
 * engine_t - cheat engine context
 * @info: engine info
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
	u32	*info;
	u32	*maxhooks;
	u32	*numhooks;
	u32	*hooklist;
	u32	*maxcodes;
	u32	*numcodes;
	u32	*codelist;
	u32	*maxcallbacks;
	u32	*callbacks;
} engine_t;

/**
 * engine_add_hook - Add a hook to an engine's hook list.
 * @engine: engine context
 * @addr: hook address
 * @val: hook value
 * @return: 0: success, -1: max hooks reached
 */
static inline int engine_add_hook(engine_t *engine, u32 addr, u32 val)
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
static inline int engine_add_code(engine_t *engine, u32 addr, u32 val)
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
static inline void engine_clear_hooks(engine_t *engine)
{
	memset(engine->hooklist, 0, *engine->numhooks * 8);
	*engine->numhooks = 0;
}

/**
 * engine_clear_codes - Clear all codes in an engine's code list.
 * @engine: engine context
 */
static inline void engine_clear_codes(engine_t *engine)
{
	memset(engine->codelist, 0, *engine->numcodes * 8);
	*engine->numcodes = 0;
}

#endif /* _ENGINE_H_ */
