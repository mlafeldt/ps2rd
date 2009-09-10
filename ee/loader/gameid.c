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

#include <libcheats.h>
#include "dbgprintf.h"
#include "gameid.h"
#include "mystring.h"

/**
 * set_game_id - Sets up the members of a game id.
 * @id: ptr to game id
 * @name: filename
 * @size: file size (greater than 0)
 * @return: GID_F_* flags showing which of the members were set
 */
int set_game_id(game_id_t *id, const char *name, size_t size)
{
	if (id != NULL) {
		memset(id, 0, sizeof(game_id_t));
		if (name != NULL) {
			strncpy(id->name, name, GID_NAME_MAX);
			id->set |= GID_F_NAME;
		}
		if (size != 0) {
			id->size = size;
			id->set |= GID_F_SIZE;
		}
		return id->set;
	}

	return GID_F_NONE;
}

/**
 * cmp_game_id - Compares two game ids.
 * @id1: ptr to 1st game id
 * @id2: ptr to 2nd game id
 * @return: GID_F_* flags showing which of the members are equal
 */
int cmp_game_id(const game_id_t *id1, const game_id_t *id2)
{
	int ret = GID_F_NONE;

	if (id1 != NULL && id2 != NULL) {
		/* Compare name */
		if ((id1->set & GID_F_NAME) && (id2->set & GID_F_NAME)) {
			if (!strcmp(id1->name, id2->name))
				ret |= GID_F_NAME;
		}
		/* Compare size */
		if ((id1->set & GID_F_SIZE) && (id2->set & GID_F_SIZE)) {
			if (id1->size == id2->size)
				ret |= GID_F_SIZE;
		}
	}

	return ret;
}

/**
 * gen_game_id - Generates the game id from a file.
 * @filename: full path to the file
 * @id: ptr to where the game id will be written to
 * @return: 0: success, <0: error
 */
int gen_game_id(const char *filename, game_id_t *id)
{
	int fd;
	size_t size;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return -1;

	/* get file size */
	size = lseek(fd, 0, SEEK_END);
	if (size < 0)
		return -1;

	/* XXX get checksum? */

	close(fd);

	set_game_id(id, filename, size);

	return 0;
}

/*
 * Parses a game title for the game ID.
 */
int get_game_id(const char *s, game_id_t *id)
{
	const char *sep = " \t";
	char buf[GAME_TITLE_MAX + 1];
	char *p = NULL;
	int i = 0;

	memset(id, 0, sizeof(game_id_t));
	strncpy(buf, s + strlen(ID_START), sizeof(buf));
	p = strtok(buf, sep);

	while (p != NULL && i < 3) {
		if (p[0] != '-') { /* '-' means that the value is not set */
			switch (i) {
			case 0: /* name */
				strncpy(id->name, p, GID_NAME_MAX);
				to_upper_str(id->name);
				id->set |= GID_F_NAME;
				D_PRINTF("id.name %s\n", id->name);
				break;

			case 1: /* size */
				if (!is_dec_str(p))
					return -1;
				if (!sscanf(p, "%i", &id->size))
					return -1;
				id->set |= GID_F_SIZE;
				D_PRINTF("id.size %i\n", id->size);
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
