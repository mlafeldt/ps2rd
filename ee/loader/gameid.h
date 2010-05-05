/*
 * gameid.h - Game ID handling
 *
 * Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
 *
 * This file is part of ps2rd, the PS2 remote debugger.
 *
 * ps2rd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ps2rd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ps2rd.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GAMEID_H_
#define _GAMEID_H_

#include <tamtypes.h>
#include <stddef.h> /* for size_t */
#include <libcheats.h>

/* Start of game id string */
#define GID_START	"/ID"

#define GID_WILDCARD	'?'

#define GID_NAME_MAX	GAME_TITLE_MAX

#define GID_F_NONE	0
#define GID_F_NAME	1
#define GID_F_SIZE	2
#define GID_F_ALL	(GID_F_NAME | GID_F_SIZE)

/**
 * gameid_t - structure to hold game id information
 * @name: name of file
 * @size: size of file in bytes
 * @set: GID_F_* flags showing which of the members are set
 */
typedef struct _game_id {
	char	name[GID_NAME_MAX + 1];
	size_t	size;
	int	set;
} gameid_t;

int gameid_set(gameid_t *id, const char *name, size_t size);
int gameid_generate(const char *filename, gameid_t *id);
int gameid_compare(const gameid_t *id1, const gameid_t *id2);
int gameid_parse(const char *s, gameid_t *id);
game_t *gameid_find(const gameid_t *id, const gamelist_t *list);

#endif /* _GAMEID_H_ */
