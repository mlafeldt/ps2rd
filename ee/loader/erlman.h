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
#include <erl.h>
#include "configman.h"
#include "engineman.h"

#define LIBKERNEL_ADDR	0x000c0000
#define ELFLDR_ADDR	0x000ff000

int erl_install_libs(const config_t *config);
int erl_install_elfldr(const config_t *config);
int erl_install_debugger(const config_t *config, engine_t *engine);

#endif /* _ERLMAN_H_ */
