/*
 * libcheats.c - Read, manipulate, and write cheat codes in text format
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 *
 * This file is part of libcheats.
 *
 * libcheats is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * libcheats is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libcheats.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include "cheatlist.h"
#include "libcheats.h"
#include "parser.h"

/**
 * cheats_init - Initialize a cheats object.
 * @cheats: cheats object
 */
void cheats_init(cheats_t *cheats)
{
	memset(cheats, 0, sizeof(cheats_t));
	GAMES_INIT(&cheats->games);
}

/**
 * cheats_destroy - Destroy a cheats object, deallocate all memory associated
 * with it, but not including the cheats_t structure itself.
 * @cheats: cheats object
 */
void cheats_destroy(cheats_t *cheats)
{
	free_games(&cheats->games);
	memset(cheats, 0, sizeof(cheats_t));
}

/**
 * cheats_read - Read and parse cheats from a stream into a cheats object.
 * @cheats: cheats object
 * @stream: stream to read cheats from
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 *
 * The functions cheats_error_text() and cheats_error_line() can be used to
 * obtain more information about an error.
 */
int cheats_read(cheats_t *cheats, FILE *stream)
{
	cheats_destroy(cheats);
	cheats_init(cheats);

	if (parse_stream(&cheats->games, stream) < 0) {
		strcpy(cheats->error_text, parse_error_text);
		cheats->error_line = parse_error_line;
		return CHEATS_FALSE;
	}

	return CHEATS_TRUE;
}

/**
 * cheats_read_file - Read and parse cheats from a text file into a cheats object.
 * @cheats: cheats object
 * @filename: name of file to read cheats from
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 *
 * The functions cheats_error_text() and cheats_error_line() can be used to
 * obtain more information about an error.
 */
int cheats_read_file(cheats_t *cheats, const char *filename)
{
	FILE *fp;
	int ret;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		sprintf(cheats->error_text, "could not open input file %s", filename);
		return CHEATS_FALSE;
	}

	ret = cheats_read(cheats, fp);
	fclose(fp);
	return ret;
}

/**
 * cheats_read_buf - Read and parse cheats from a text buffer into a cheats object.
 * @cheats: cheats object
 * @buf: buffer holding text (must be NUL-terminated!)
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 *
 * The functions cheats_error_text() and cheats_error_line() can be used to
 * obtain more information about an error.
 */
int cheats_read_buf(cheats_t *cheats, const char *buf)
{
	cheats_destroy(cheats);
	cheats_init(cheats);

	if (parse_buf(&cheats->games, buf) < 0) {
		strcpy(cheats->error_text, parse_error_text);
		cheats->error_line = parse_error_line;
		return CHEATS_FALSE;
	}

	return CHEATS_TRUE;
}

/**
 * cheats_write - Write cheats from a cheats object to a stream.
 * @cheats: cheats object
 * @stream: stream to write cheats to
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 */
void cheats_write(cheats_t *cheats, FILE *stream)
{
	game_t *game;
	cheat_t *cheat;
	code_t *code;

	GAMES_FOREACH(game, &cheats->games) {
		fprintf(stream, "\"%s\"\n", game->title);
		CHEATS_FOREACH(cheat, &game->cheats) {
			fprintf(stream, "%s\n", cheat->desc);
			CODES_FOREACH(code, &cheat->codes) {
				fprintf(stream, "%08X %08X\n", code->addr, code->val);
			}
		}
		fprintf(stream, "\n//--------\n\n");
	}
}

/**
 * cheats_write_file - Write cheats from a cheats object to a text file.
 * @cheats: cheats objects
 * @filename: name of file to write cheats to
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 *
 * The function cheats_error_text() can be used to obtain more information about
 * an error.
 */
int cheats_write_file(cheats_t *cheats, const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "w");
	if (fp == NULL) {
		sprintf(cheats->error_text, "could not open output file %s", filename);
		return CHEATS_FALSE;
	}

	cheats_write(cheats, fp);
	fclose(fp);
	return CHEATS_TRUE;
}

/**
 * cheats_error_text - Return the text of the parse error.
 * @cheats: cheats object
 * @return: error text
 */
const char *cheats_error_text(const cheats_t *cheats)
{
	return cheats->error_text;
}

/**
 * cheats_error_line - Return the line number of the last parse error.
 * @cheats: cheats object
 * @return: line number
 */
int cheats_error_line(const cheats_t *cheats)
{
	return cheats->error_line;
}
