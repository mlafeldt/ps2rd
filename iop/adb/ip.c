/*
 * ip.c - lightweight IP implementation
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

#include "ip.h"
#include "inet.h"
#include "udp.h"
#include "linux/if_ether.h"
#include "linux/ip.h"

typedef struct {
	/* Ethernet header (14) */
	struct ethhdr eth;

	/* IP header (20) */
	struct iphdr ip;
} ip_pkt_t __attribute__((packed));

/*
 * ip_input: Called from smap RX intr handler when a IP ethernet
 * frame is received. (careful with Intr context)
 */
void ip_input(void *buf, int size)
{
	ip_pkt_t *ip_pkt = (ip_pkt_t *)buf;

	if ((ip_pkt->ip.version == IPVERSION) && (ip_pkt->ip.ihl == 5)) { /* ihl is 5 words */

		if (ip_pkt->ip.frag_off == 0) { /* drop IP fragments */

			if (inet_chksum(&ip_pkt->ip, 20) == 0) { /* check IP checksum */

				if (ip_pkt->ip.protocol == 0x11) /* filter UDP packet */
					udp_input(buf, size);
			}
		}
	}
}
