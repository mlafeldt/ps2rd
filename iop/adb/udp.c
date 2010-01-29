/*
 * udp.c - Advanced debugger
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

#include "udp.h"
#include "inet.h"
#include "smap.h"
#include "linux/if_ether.h"
#include "linux/ip.h"

#define M_PRINTF(format, args...) \
	printf(ADB_MODNAME ": " format, ## args)

static u32 g_ip_addr_dst;
static u32 g_ip_addr_src;
static u16 g_ip_port_dst;
static u16 g_ip_port_src;

static u8 udp_sndbuf[ETH_FRAME_LEN+8] __attribute__((aligned(64))); /* 1 MTU(1514) max */
static u8 *p_rcptbuf;
static int rcptbuf_size;
static int udp_mutex = -1;

#define	UDP_TIMEOUT	(3000*1000)
iop_sys_clock_t timeout_sysclock;
static int timeout_flag = 0;

/*
 * Timer Interrupt handler for UDP reply timeout (Intr context)
 * will be used later for aknowlegment of packets we send to the
 * PC, and retransmit packet if needed.
 */
static unsigned int timer_intr_handler(void *args)
{
	if (timeout_flag)
		iSignalSema(udp_mutex);

	iSetAlarm(&timeout_sysclock, timer_intr_handler, NULL);

	return (unsigned int)args;
}

/*
 * udp_init: initialize UDP layer
 */
void udp_init(g_param_t *g_param)
{
	udp_pkt_t *udp_pkt;

	/* Initialize the static elements of our UDP packet */
	udp_pkt = (udp_pkt_t *)udp_sndbuf;

	memcpy(udp_pkt->eth.h_dest, g_param->eth_addr_dst, ETH_ALEN*2);
	udp_pkt->eth.h_proto = HTONS(ETH_P_IP);

	udp_pkt->ip.version = IPVERSION;
	udp_pkt->ip.ihl = sizeof(struct iphdr) / 4;
	udp_pkt->ip.tos = 0;
	udp_pkt->ip.id = 0;
	udp_pkt->ip.frag_off = 0;
	udp_pkt->ip.ttl = IPDEFTTL;
	udp_pkt->ip.protocol = 0x11;
	memcpy(udp_pkt->ip.saddr, &g_param->ip_addr_src, 4);
	memcpy(udp_pkt->ip.daddr, &g_param->ip_addr_dst, 4);

	udp_pkt->udp_port_src = g_param->ip_port_src;
	udp_pkt->udp_port_dst = g_param->ip_port_dst;

	/* create a mutex for catching UDP reply */
	udp_mutex = CreateMutex(IOP_MUTEX_LOCKED);
	g_param->rcv_mutex = udp_mutex;

	/* install a timer for UDP reply timeout */
	USec2SysClock(UDP_TIMEOUT, &timeout_sysclock);
	SetAlarm(&timeout_sysclock, timer_intr_handler, NULL);

	/* these are stored in network byte order, careful later */
	g_ip_addr_dst = g_param->ip_addr_dst;
	g_ip_addr_src = g_param->ip_addr_src;
	g_ip_port_dst = g_param->ip_port_dst;
	g_ip_port_src = g_param->ip_port_src;
}

/*
 * udp_output: send an UDP ethernet frame
 */
int udp_output(void *buf, int size)
{
	udp_pkt_t *udp_pkt;
	int pktsize, udpsize;
	int oldstate;

	if ((size + sizeof(udp_pkt_t)) > sizeof(udp_sndbuf))
		size = sizeof(udp_sndbuf) - sizeof(udp_pkt_t);

	udp_pkt = (udp_pkt_t *)udp_sndbuf;
	pktsize = size + sizeof(udp_pkt_t);

	udp_pkt->ip.tot_len = htons(pktsize - ETH_HLEN);		/* Subtract the ethernet header size */

	udp_pkt->ip.check = 0;
	udp_pkt->ip.check = inet_chksum(&udp_pkt->ip, 20);	/* Checksum the IP header (20 bytes) */

	udpsize = htons(size + 8);							/* Size of the UDP header + data */
	udp_pkt->udp_len = udpsize;
	memcpy(udp_sndbuf + sizeof(udp_pkt_t), buf, size);

	CpuSuspendIntr(&oldstate);

	udp_pkt->udp_csum = 0;
	udp_pkt->udp_csum = inet_chksum_pseudo(&udp_sndbuf[ETH_HLEN+20], &g_ip_addr_src, &g_ip_addr_dst, 8+size);

	/* send the ethernet frame */
	while (smap_xmit(udp_pkt, pktsize) != 0);

	CpuResumeIntr(oldstate);

	return 0;
}

/*
 * udp_input: Called from smap RX intr handler when a UDP ethernet
 * frame is received. (careful with Intr context)
 */
int udp_input(void *buf, int size)
{
	if (udp_mutex >= 0) {
		udp_pkt_t *udp_pkt = (udp_pkt_t *)buf;

		/* check the packet is coming to our local port */
		if (udp_pkt->udp_port_dst == g_ip_port_src) {

			/* check pseudo checksum */
			if (inet_chksum_pseudo(&udp_pkt->udp_port_src, &g_ip_addr_dst, &g_ip_addr_src, (ntohs(udp_pkt->ip.tot_len)-20)) == 0) {
				p_rcptbuf = buf;
				rcptbuf_size = size;
				iSignalSema(udp_mutex);
			}
		}
	}

	return 1;
}

/*
 * udp_getpacket: copy UDP packet from smap rcv buffer
 */
void udp_getpacket(void *buf, int *size)
{
	int oldstate;
	register int pktsize;

	CpuSuspendIntr(&oldstate);

	/* get packet data pointer and packet size */
	udp_pkt_t *udp_pkt = (udp_pkt_t *)p_rcptbuf;
	pktsize = ntohs(udp_pkt->udp_len) - 8; /* substract udp header */

	p_rcptbuf[ETH_HLEN+20+8+pktsize] = 0;
	memcpy(buf, &p_rcptbuf[ETH_HLEN+20+8], pktsize+1);
	*size = pktsize;

	CpuResumeIntr(oldstate);
}
