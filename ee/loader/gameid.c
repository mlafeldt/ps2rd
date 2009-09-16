/*
 * gameid.c - Game ID handling
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
#include <libcheats.h>
#include "dbgprintf.h"
#include "gameid.h"
#include "mystring.h"

/**
 * gameid_set - Set up the members of a game id.
 * @id: ptr to game id
 * @name: filename
 * @size: file size (greater than 0)
 * @return: GID_F_* flags showing which of the members were set
 */
int gameid_set(gameid_t *id, const char *name, size_t size)
{
	if (id != NULL) {
		memset(id, 0, sizeof(gameid_t));
		if (name != NULL) {
			strncpy(id->name, name, GID_NAME_MAX);
			id->set |= GID_F_NAME;
		}
		if (size > 0) {
			id->size = size;
			id->set |= GID_F_SIZE;
		}
		return id->set;
	}

	return GID_F_NONE;
}

/**
 * gameid_generate - Generate the game id from a file.
 * @filename: full path to the file
 * @id: ptr to where the game id will be written to
 * @return: 0: success, <0: error
 */
int gameid_generate(const char *filename, gameid_t *id)
{
	int fd;
	size_t size;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return -1;

	/* get file size */
	size = lseek(fd, 0, SEEK_END);
	close(fd);
	if (size < 0)
		return -1;

	/* XXX get checksum? */

	gameid_set(id, filename, size);

	return 0;
}

/**
 * gameid_compare - Compare two game ids.
 * @id1: ptr to 1st game id
 * @id2: ptr to 2nd game id
 * @return: 0: equal, <0: unequal
 */
int gameid_compare(const gameid_t *id1, const gameid_t *id2)
{
	int match = GID_F_NONE;

	if (id1 != NULL && id2 != NULL) {
		/* compare name */
		if ((id1->set & GID_F_NAME) && (id2->set & GID_F_NAME)) {
			if (!strstr_wc(id1->name, id2->name, GID_WILDCARD))
				match |= GID_F_NAME;
			else
				return -1;
		}
		/* compare size */
		if ((id1->set & GID_F_SIZE) && (id2->set & GID_F_SIZE)) {
			if (id1->size == id2->size)
				match |= GID_F_SIZE;
			else
				return -1;
		}
	}

	return (match != GID_F_NONE) ? 0 : -1;
}

/**
 * gameid_parse - Parse a string for a game ID.
 * @s: string to be parsed
 * @id: ptr to game id
 * @return: 0: success, <0: error
 */
int gameid_parse(const char *s, gameid_t *id)
{
	const char *sep = " \t";
	char buf[256];
	char *p = NULL;
	int i = 0;

	memset(id, 0, sizeof(gameid_t));

	p = strstr(s, GID_START);
	if (p == NULL)
		return -1;

	strncpy(buf, p + strlen(GID_START), sizeof(buf) - 1);
	p = strtok(buf, sep);

	while (p != NULL && i < 3) {
		if (p[0] != '-') { /* '-' means that the value is not set */
			switch (i) {
			case 0: /* name */
				strncpy(id->name, p, GID_NAME_MAX);
				id->set |= GID_F_NAME;
				break;

			case 1: /* size */
				if (!is_dec_str(p))
					return -1;
				if (!sscanf(p, "%i", &id->size))
					return -1;
				id->set |= GID_F_SIZE;
				break;

			case 2: /* XXX checksum? */
				break;
			}
		}

		i++;
		p = strtok(NULL, sep);
	}

	return 0;
}

/**
 * gameid_find - Find cheats for a game by its game ID.
 * @id: ptr to game id
 * @list: game list to be searched
 * @return: ptr to game, or NULL if it could not be found
 */
game_t *gameid_find(const gameid_t *id, const gamelist_t *list)
{
	game_t *game;
	gameid_t id2;

	GAMES_FOREACH(game, list) {
		if (!gameid_parse(game->title, &id2)) {
			if (!gameid_compare(id, &id2))
				return game;
		}
	}

	return NULL;
}
