/*
 * mycdvd.c - CDVD wrapper and helper functions
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
#include <kernel.h>
#include <fileio.h>
#include <string.h>
#include "mycdvd.h"
#include "dbgprintf.h"

#if 0
int cdOpenTray(void)
{
	u32 traychk;
	int ret;

	D_PRINTF("Opening disc tray... ");
	ret = cdTrayReq(CDVD_TRAY_OPEN, &traychk);
	D_PRINTF(ret == 0 ? "Failed.\n" : "OK.\n");

	return ret;
}

int cdCloseTray(void)
{
	u32 traychk;
	int ret;

	D_PRINTF("Closing disc tray... ");
	ret = cdTrayReq(CDVD_TRAY_CLOSE, &traychk);
	D_PRINTF(ret == 0 ? "Failed.\n" : "OK.\n");

	return ret;
}

void cdToggleTray(void)
{
	if (cdStatus() == CDVD_STAT_OPEN) {
		cdCloseTray();
		cdDiskReady(CDVD_NOBLOCK);
	} else {
		cdOpenTray();
	}
}

int cdIsPS2Game(void)
{
	switch (cdGetDiscType()) {
		case CDVD_TYPE_PS2CD:
		case CDVD_TYPE_PS2CDDA:
		case CDVD_TYPE_PS2DVD:
			return 1;
		default:
			return 0;
	}
}
#endif

/**
 * cdGetElf - Parse "cdrom0:\\SYSTEM.CNF;1" for ELF filename.
 * @elfname: ptr to where filename is written to
 * @return: 0: success, <0: error
 */
int cdGetElf(char *elfname)
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
		D_PRINTF("%s: Can't open 'SYSTEM.CNF'\n", __FUNCTION__);
		return -2;
	}

	n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (!n) {
		D_PRINTF("%s: File size = 0\n", __FUNCTION__);
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
		D_PRINTF("%s: Can't find %s entry\n", __FUNCTION__, entry);
		return -4;
	}

	return 0;
}

/**
 * cdRunElf - Run a PS2 game from CD/DVD.
 * @return: does not return on success, -1: error
 */
int cdRunElf(void)
{
	char elfname[FIO_PATH_MAX];

	/* Spin up drive */
	cdDiskReady(CDVD_BLOCK);
	cdStandby();
	cdSync(CDVD_BLOCK);

	/* Get ELF filename and execute it */
	if (!cdGetElf(elfname)) {
		D_PRINTF("Running ELF %s ...\n", elfname);
		LoadExecPS2(elfname, 0, NULL);
	}

	cdStop();
	cdSync(CDVD_BLOCK);
	return -1;
}
