/*
 * Manage cheat codes
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

#ifndef _CHEATMAN_H_
#define _CHEATMAN_H_

#include <libcheats.h>
#include "engine.h"

int load_cheats(const char *cheatfile, cheats_t *cheats);
game_t *find_cheats(const char *boot2, const cheats_t *cheats);
void activate_cheats(const game_t *game, engine_t *engine);
void reset_cheats(engine_t *engine);

#endif /* _CHEATMAN_H_ */
