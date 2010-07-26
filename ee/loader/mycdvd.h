/*
 * CDVD wrapper and helper functions
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

#ifndef _MYCDVD_H_
#define _MYCDVD_H_

#include <tamtypes.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>
#include <libcdvd.h>
#include "dbgprintf.h"

#define CDVD_BLOCK	0
#define CDVD_NOBLOCK	1

static inline void _cdStandby(int mode)
{
	cdDiskReady(mode);
	cdStandby();
	cdSync(mode);
}

static inline void _cdStop(int mode)
{
	cdDiskReady(mode);
	cdStop();
	cdSync(mode);
}

/**
 * cdGetElf - Parse "cdrom0:\\SYSTEM.CNF;1" for ELF filename.
 * @elfname: ptr to where filename is written to
 * @return: 0: success, <0: error
 */
static inline int cdGetElf(char *elfname)
{
	const char *sep = "\n";
	const char *entry = "BOOT2";
	char buf[256];
	char *ptr, *tok;
	int fd, n, found = 0;

	if (elfname == NULL)
		return -1;

	fd = open("cdrom0:\\SYSTEM.CNF;1", O_TEXT | O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "%s: Can't open 'SYSTEM.CNF'\n", __FUNCTION__);
		return -2;
	}

	n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (!n) {
		fprintf(stderr, "%s: File size = 0\n", __FUNCTION__);
		return -3;
	}

	buf[n] = '\0';
	tok = strtok(buf, sep);

	while (tok != NULL) {
		D_PRINTF("%s: %s\n", __FUNCTION__, tok);

		if (!found) {
			ptr = strstr(tok, entry);
			if (ptr != NULL) {
				ptr += strlen(entry);
				while (isspace(*ptr) || (*ptr == '='))
					ptr++;
				strcpy(elfname, ptr);
				found = 1;
			}
		}

		tok = strtok(NULL, sep);
	}

	if (!found) {
		fprintf(stderr, "%s: Can't find %s entry\n", __FUNCTION__, entry);
		return -4;
	}

	return 0;
}

#endif /* _MYCDVD_H_ */
