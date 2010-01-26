/*
 * udp.h - Advanced debugger
 *
 * Copyright (C) 2009-2010 jimmikaelkael <jimmikaelkael@wanadoo.fr>
 *
 * This file is part of ps2rd, the PS2 remote debugger.
 *
 * ps2rd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ps2rd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ps2rd.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _IOP_UDP_H_
#define _IOP_UDP_H_

#include <tamtypes.h>
#include <irx.h>

#include <intrman.h>
#include <thbase.h>
#include <thsemap.h>
#include <sysclib.h>
#include <stdio.h>

#include "adb.h"

typedef struct {
	/* Ethernet header (14) */
	eth_hdr_t eth;

	/* IP header (20) */
	ip_hdr_t ip;

	/* UDP header (8) */
	u16	udp_port_src;
	u16	udp_port_dst;
	u16	udp_len;
	u16	udp_csum;

	/* Data goes here */
} udp_pkt_t __attribute__((packed));

void udp_init(g_param_t *g_param);
int udp_input(void *buf, int size);
int udp_output(void *buf, int size);
void udp_getpacket(void *buf, int *size);

#endif /* _IOP_UDP_H_ */
