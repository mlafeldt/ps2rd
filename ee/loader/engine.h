/*
 * Interface to cheat engine
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

#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <tamtypes.h>

/* cheat engine context */
typedef struct {
	int	(*get_max_hooks)(void);
	int	(*get_num_hooks)(void);
	int	(*add_hook)(u32 addr, u32 val);
	void	(*clear_hooks)(void);

	int	(*get_max_codes)(void);
	void	(*set_max_codes)(int num);
	int	(*get_num_codes)(void);
	int	(*add_code)(u32 addr, u32 val);
	void	(*clear_codes)(void);

	int	(*register_callback)(void *func);
} engine_t;

#endif /* _ENGINE_H_ */
