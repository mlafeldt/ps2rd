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

#include <tamtypes.h>
#include "inet.h"
#include "eth.h"
#include "udp.h"
#include "linux/if_ether.h"
#include "linux/ip.h"

typedef struct {
	/* Ethernet header (14) */
	struct ethhdr eth;

	/* IP header (20) */
	struct iphdr ip;
} ip_pkt_t __attribute__((packed));

static u32 g_ip_addr_dst;
static u32 g_ip_addr_src;

/*
 * ip_init: Init IP protocol layer
 */
void ip_init(u32 ip_addr_dst, u32 ip_addr_src)
{
	/* these are stored in network byte order, careful later */
	g_ip_addr_dst = ip_addr_dst;
	g_ip_addr_src = ip_addr_src;
}

/*
 * ip_input: Called from ethernet layer when an IP packet
 * is received. (careful with Intr context)
 */
void ip_input(void *buf, int size)
{
	ip_pkt_t *ip_pkt = (ip_pkt_t *)buf;

	if (ip_pkt->ip.version == IPVERSION &&
			ip_pkt->ip.ihl == (sizeof(struct iphdr) / 4) &&
			!(ip_pkt->ip.frag_off & 0x3f) && 	/* drop IP fragments */
			!(ip_pkt->ip.frag_off & 0xff00) && 	/* drop IP fragments */
			!inet_chksum(&ip_pkt->ip, sizeof(struct iphdr)) && /* verify IP checksum */
			ip_pkt->ip.protocol == 0x11) /* filter UDP packet */
		udp_input(buf, size);
}

/*
 * ip_output: Send an IP packet to ethernet layer
 */
int ip_output(void *buf, int size, u8 ip_protocol)
{
	ip_pkt_t *ip_pkt = (ip_pkt_t *)buf;

	ip_pkt->ip.version = IPVERSION;
	ip_pkt->ip.ihl = sizeof(struct iphdr) / 4;
	ip_pkt->ip.tos = 0;
	ip_pkt->ip.id = 0;
	ip_pkt->ip.frag_off = 0;
	ip_pkt->ip.ttl = IPDEFTTL;
	ip_pkt->ip.protocol = ip_protocol;
	memcpy(ip_pkt->ip.saddr, &g_ip_addr_src, 4);
	memcpy(ip_pkt->ip.daddr, &g_ip_addr_dst, 4);

	size += sizeof(struct iphdr);
	ip_pkt->ip.tot_len = htons(size);

	ip_pkt->ip.check = 0;
	ip_pkt->ip.check = inet_chksum(&ip_pkt->ip, sizeof(struct iphdr));	/* Checksum the IP header (20 bytes) */

	return eth_output(buf, size+ETH_HLEN, HTONS(ETH_P_IP));
}
