/*
 * memdisk.c
 *
 * Copyright (C) 2009-2010 jimmikaelkael <jimmikaelkael@wanadoo.fr>
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
 */

#include <stdio.h>
#include <ioman.h>
#include <errno.h>
#include <io_common.h>

const char dev_name[] = "MDISK\0";

/* img driver ops functions prototypes */
int mdisk_dummy(void);
int mdisk_read(iop_file_t *f, void *buf, u32 size);
int mdisk_lseek(iop_file_t *f, u32 pos, int where);

/* driver ops func tab */
void *mdiskdev_ops[10] = {
	(void*)mdisk_dummy,
	(void*)mdisk_dummy,
	NULL,
	(void*)mdisk_dummy,
	(void*)mdisk_dummy,
	(void*)mdisk_read,
	NULL,
	(void*)mdisk_lseek,
	NULL,
	NULL
};

/* driver descriptor */
static iop_device_t mdiskdev = {
	dev_name,
	IOP_DT_FS,
	1,
	dev_name,
	(struct _iop_device_ops *)&mdiskdev_ops
};

typedef struct {
	void *buf;
	u32 pad1;
	u32 pad2;
	int size;
} ioprp_img_t;

static ioprp_img_t ioprpimg = {
	(void *)NULL,
	0,
	0,
	0
};

IRX_ID(dev_name, 1, 1);

/*
 * module entry point
 */
int _start(int argc, char** argv)
{
	register int r;

	r = AddDrv(&mdiskdev);

	return r >> 31;
}

/*
 * mdisk_dummy - dummy function for some driver ops
 */
int mdisk_dummy(void)
{
	return 0;
}

/*
 * mdisk_read - IOPRP read op
 */
int mdisk_read(iop_file_t *f, void *buf, u32 size)
{
	register int i;
	void *ioprp_img;

	ioprp_img = ioprpimg.buf;

	for (i = size; i > 0; i -= 4) {
		*((u32 *)buf) = *((u32 *)ioprp_img);
		buf += 4;
		ioprp_img += 4;
	}

	return size;
}

/*
 * mdisk_lseek - IOPRP seek op
 */
int mdisk_lseek(iop_file_t *f, u32 pos, int where)
{
	if (where == SEEK_SET)
		return 0;

	return ioprpimg.size;
}
