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

#include <tamtypes.h>
#include <string.h>
#include <stdio.h>
#include "elfid.h"

int elfid_set(elfid_t *id, const char *name, size_t size)
{
	if (id != NULL) {
		memset(id, 0, sizeof(elfid_t));
		if (name != NULL) {
			strncpy(id->name, name, ELFID_NAME_MAX);
			id->set |= ELFID_F_NAME;
		}
		if (size > 0) {
			id->size = size;
			id->set |= ELFID_F_SIZE;
		}
		return id->set;
	}

	return ELFID_F_NONE;
}

int elfid_generate(const char *filename, elfid_t *id)
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

	elfid_set(id, filename, size);

	return 0;
}

static int strncmp_wc(const char *s1, const char *s2, size_t n, int wc)
{
	unsigned char a, b;

	while (n-- > 0) {
		a = (unsigned char)*s1++;
		b = (unsigned char)*s2++;
		if (a != b && a != wc && b != wc)
			return a - b;
		if (!a)
			return 0;
	}

	return 0;
}

static char *strstr_wc(const char *haystack, const char *needle, int wc)
{
	char *pos;

	if (!strlen(needle))
		return (char*)haystack;

	pos = (char*)haystack;

	while (*pos) {
		if (!strncmp_wc(pos, needle, strlen(needle), wc))
			return pos;
		pos++;
	}

	return NULL;
}

int elfid_compare(const elfid_t *id1, const elfid_t *id2)
{
	int match = ELFID_F_NONE;

	if (id1 != NULL && id2 != NULL) {
		/* compare name */
		if ((id1->set & ELFID_F_NAME) && (id2->set & ELFID_F_NAME)) {
			if (strstr_wc(id1->name, id2->name, ELFID_WILDCARD))
				match |= ELFID_F_NAME;
			else
				return -1;
		}
		/* compare size */
		if ((id1->set & ELFID_F_SIZE) && (id2->set & ELFID_F_SIZE)) {
			if (id1->size == id2->size)
				match |= ELFID_F_SIZE;
			else
				return -1;
		}
	}

	return (match != ELFID_F_NONE) ? 0 : -1;
}

int elfid_parse(const char *s, elfid_t *id)
{
	const char *sep = " \t";
	char buf[ELFID_NAME_MAX + 1];
	char *p = NULL;
	int i = 0;

	memset(id, 0, sizeof(elfid_t));

	strncpy(buf, s, ELFID_NAME_MAX);
	p = strtok(buf, sep);

	while (p != NULL && i < 3) {
		if (p[0] != '-') { /* '-' means that the value is not set */
			switch (i) {
			case 0: /* name */
				strncpy(id->name, p, ELFID_NAME_MAX);
				id->set |= ELFID_F_NAME;
				break;

			case 1: /* size */
				if (!sscanf(p, "%i", &id->size))
					return -1;
				id->set |= ELFID_F_SIZE;
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
