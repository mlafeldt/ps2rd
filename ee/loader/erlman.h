/*
 * erlman.h - ERL file manager
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

#ifndef _ERLMAN_H_
#define _ERLMAN_H_

#include <tamtypes.h>
#include "configman.h"

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

int install_erls(const config_t *config, engine_t *engine);
void uninstall_erls(void);

int engine_add_hook(engine_t *engine, u32 addr, u32 val);
int engine_add_code(engine_t *engine, u32 addr, u32 val);
void engine_clear_hooks(engine_t *engine);
void engine_clear_codes(engine_t *engine);

#endif /* _ERLMAN_H_ */
