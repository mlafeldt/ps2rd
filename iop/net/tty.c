/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: tty.c 629 2004-10-11 00:45:00Z mrbrown $
# TTY filesystem for UDPTTY.
*/

#include <tamtypes.h>
#include <ioman.h>
#include <thsemap.h>
#include <sysclib.h>
#include <errno.h>

#include "ip.h"
#include "udp.h"

#define DEVNAME "tty"
#define TTY_LOCAL_PORT			IP_PORT(18194)

extern iop_device_t tty_device;
static int tty_sema = -1;

static u16 g_ip_port_log;

/* Init TTY */
void udptty_init(u16 ip_port_log)
{
	close(0);
	close(1);
	DelDrv(DEVNAME);

	if (AddDrv(&tty_device) < 0)
		return;

	open(DEVNAME "00:", 0x1000|O_RDWR);
	open(DEVNAME "00:", O_WRONLY);

	g_ip_port_log = ip_port_log;
}

/* TTY driver.  */

static int tty_init(iop_device_t *device)
{
	if ((tty_sema = CreateMutex(IOP_MUTEX_UNLOCKED)) < 0)
		return -1;

	return 0;
}

static int tty_deinit(iop_device_t *device)
{
	DeleteSema(tty_sema);
	return 0;
}

static int tty_stdout_fd(void) { return 1; }

static int tty_write(iop_file_t *file, void *buf, size_t size)
{
	int res = 0;

	WaitSema(tty_sema);
	res = udp_output(TTY_LOCAL_PORT, g_ip_port_log, buf, size);

	SignalSema(tty_sema);
	return res;
}

static int tty_error(void) { return -EIO; }

static iop_device_ops_t tty_ops = { tty_init, tty_deinit, (void *)tty_error,
	(void *)tty_stdout_fd, (void *)tty_stdout_fd, (void *)tty_error,
	(void *)tty_write, (void *)tty_error, (void *)tty_error,
	(void *)tty_error, (void *)tty_error, (void *)tty_error,
	(void *)tty_error, (void *)tty_error, (void *)tty_error,
	(void *)tty_error,  (void *)tty_error };

iop_device_t tty_device =
{ DEVNAME, IOP_DT_CHAR|IOP_DT_CONS, 1, "TTY via SMAP UDP", &tty_ops };
