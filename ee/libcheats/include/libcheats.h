/*
 * libcheats.h - Read, manipulate, and write cheat codes in text format
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

#ifndef _LIBCHEATS_H_
#define _LIBCHEATS_H_

#include <stdio.h>
#include "cheatlist.h"

/* Version information */
#define LIBCHEATS_VERSION	"0.1"
#define LIBCHEATS_VERNUM	0x0001
#define LIBCHEATS_VER_MAJOR	0
#define LIBCHEATS_VER_MINOR	1

/* Return values */
#define CHEATS_TRUE		0
#define CHEATS_FALSE		(-1)

/**
 * cheats_t - cheats object
 * @games: list of games
 * @error_text: text of last error
 * @error_line: line no. of last parse error
 */
typedef struct _cheats {
	gamelist_t	games;
	char		error_text[256];
	int		error_line;
} cheats_t;

/**
 * cheats_init - Initialize a cheats object.
 * @cheats: cheats object
 */
extern void cheats_init(cheats_t *cheats);

/**
 * cheats_destroy - Destroy a cheats object, deallocate all memory associated
 * with it, but not including the cheats_t structure itself.
 * @cheats: cheats object
 */
extern void cheats_destroy(cheats_t *cheats);

/**
 * cheats_read - Read and parse cheats from a stream into a cheats object.
 * @cheats: cheats object
 * @stream: stream to read cheats from
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 *
 * The functions cheats_error_text() and cheats_error_line() can be used to
 * obtain more information about an error.
 */
extern int cheats_read(cheats_t *cheats, FILE *stream);

/**
 * cheats_read_file - Read and parse cheats from a text file into a cheats object.
 * @cheats: cheats object
 * @filename: name of file to read cheats from
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 *
 * The functions cheats_error_text() and cheats_error_line() can be used to
 * obtain more information about an error.
 */
extern int cheats_read_file(cheats_t *cheats, const char *filename);

/**
 * cheats_read_buf - Read and parse cheats from a text buffer into a cheats object.
 * @cheats: cheats object
 * @buf: buffer holding text (must be NUL-terminated!)
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 *
 * The functions cheats_error_text() and cheats_error_line() can be used to
 * obtain more information about an error.
 */
extern int cheats_read_buf(cheats_t *cheats, const char *buf);

/**
 * cheats_write - Write cheats from a cheats object to a stream.
 * @cheats: cheats object
 * @stream: stream to write cheats to
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 */
extern void cheats_write(cheats_t *cheats, FILE *stream);

/**
 * cheats_write_file - Write cheats from a cheats object to a text file.
 * @cheats: cheats objects
 * @filename: name of file to write cheats to
 * @return: CHEATS_TRUE: success, CHEATS_FALSE: error
 *
 * The function cheats_error_text() can be used to obtain more information about
 * an error.
 */
extern int cheats_write_file(cheats_t *cheats, const char *filename);

/**
 * cheats_error_text - Return the text of the last error.
 * @cheats: cheats object
 * @return: error text
 */
extern const char *cheats_error_text(const cheats_t *cheats);

/**
 * cheats_error_line - Return the line number of the last parse error.
 * @cheats: cheats object
 * @return: line number
 */
extern int cheats_error_line(const cheats_t *cheats);

#endif /* _LIBCHEATS_H_ */
