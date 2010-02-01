/*
 * eth.c - lightweight Ethernet implementation
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

#include <tamtypes.h>
#include "inet.h"
#include "smap.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"
#include "tty.h"
#include "linux/if_ether.h"

static u8 g_eth_addr_dst[ETH_ALEN];
static u8 g_eth_addr_src[ETH_ALEN];

/*
 * eth_init: initialize Ethernet layer, eth_addr_src will
 * be filled with PS2 ethernet address
 */
int eth_init(u32 ip_addr_dst, u32 ip_addr_src)
{
	register int r;

	/* init SMAP driver */
	r = smap_init(g_eth_addr_src);

	/* prepare ethernet addresses for broadcast */
	memset(g_eth_addr_dst, 0xff, ETH_ALEN);

	/* init ARP layer */
	arp_init(g_eth_addr_dst, g_eth_addr_src, ip_addr_dst, ip_addr_src);

	/* init IP layer */
	ip_init(ip_addr_dst, ip_addr_src);

	/* init UDP layer */
	udp_init(ip_addr_dst, ip_addr_src);

	/* init UDP TTY */
#ifdef UDPTTY
	udptty_init(IP_PORT(7411));
#endif

	return r;
}

/*
 * eth_output: output an ethernet frame to hardware
 */
int eth_output(void *buf, int size, int h_proto)
{
	struct ethhdr *eth = (struct ethhdr *)buf;

	/* fill ethernet header */
	memcpy(eth->h_dest, g_eth_addr_dst, ETH_ALEN);
	memcpy(eth->h_source, g_eth_addr_src, ETH_ALEN);
	eth->h_proto = h_proto;

	return smap_xmit(buf, size);
}

/*
 * eth_input: Called from smap RX intr handler when an Ethernet
 * frame is received. (careful with Intr context)
 */
void eth_input(void *buf, int size)
{
	struct ethhdr *eth = (struct ethhdr *)buf;

	if (eth->h_proto == NTOHS(ETH_P_IP)) /* the ethernet frame contains an IP packet */
		ip_input(buf, size);

	else if (eth->h_proto == NTOHS(ETH_P_ARP)) /* the ethernet frame contains an ARP packet */
		arp_input(buf, size);
}
