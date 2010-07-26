/*
 * ELF ID
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

#ifndef _ELFID_H_
#define _ELFID_H_

#include <tamtypes.h>
#include <stddef.h> /* for size_t */

#define ELFID_WILDCARD	'?'

#define ELFID_NAME_MAX	255

#define ELFID_F_NONE	0
#define ELFID_F_NAME	1
#define ELFID_F_SIZE	2
#define ELFID_F_ALL	(ELFID_F_NAME | ELFID_F_SIZE)

/**
 * elfid_t - structure to hold elf id information
 * @name: name of file
 * @size: size of file in bytes
 * @set: ELFID_F_* flags showing which of the members are set
 */
typedef struct _elfid {
	char	name[ELFID_NAME_MAX + 1];
	size_t	size;
	int	set;
} elfid_t;

/**
 * elfid_set - Set up the members of a elf id.
 * @id: ptr to elf id
 * @name: filename
 * @size: file size (greater than 0)
 * @return: ELFID_F_* flags showing which of the members were set
 */
int elfid_set(elfid_t *id, const char *name, size_t size);

/**
 * elfid_generate - Generate the elf id from a file.
 * @filename: full path to the file
 * @id: ptr to where the elf id will be written to
 * @return: 0: success, <0: error
 */
int elfid_generate(const char *filename, elfid_t *id);

/**
 * elfid_compare - Compare two elf ids.
 * @id1: ptr to 1st elf id
 * @id2: ptr to 2nd elf id
 * @return: 0: equal, -1: unequal
 */
int elfid_compare(const elfid_t *id1, const elfid_t *id2);

/**
 * elfid_parse - Parse a string for a elf id.
 * @s: string to be parsed
 * @id: ptr to elf id
 * @return: 0: success, -1: error
 */
int elfid_parse(const char *s, elfid_t *id);

#endif /* _ELFID_H_ */
