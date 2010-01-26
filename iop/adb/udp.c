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

#define M_PRINTF(format, args...) \
	printf(ADB_MODNAME ": " format, ## args)

static u8 udp_sndbuf[1514] __attribute__((aligned(64))); /* 1 MTU */
static u8 udp_rcvbuf[1514] __attribute__((aligned(64))); /* 1 MTU */
static void *p_rcptbuf;
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
void udp_init(void)
{
	udp_pkt_t *udp_pkt;

	/* Initialize the static elements of our UDP packet */
	udp_pkt = (udp_pkt_t *)udp_sndbuf;

	memcpy(udp_pkt->eth.addr_dst, g_param.eth_addr_dst, 12);
	udp_pkt->eth.type = 0x0008;	/* Network byte order: 0x800 */

	udp_pkt->ip.hlen = 0x45;
	udp_pkt->ip.tos = 0;
	udp_pkt->ip.id = 0;
	udp_pkt->ip.flags = 0;
	udp_pkt->ip.frag_offset = 0;
	udp_pkt->ip.ttl = 64;
	udp_pkt->ip.proto = 0x11;
	memcpy(&udp_pkt->ip.addr_src.addr, &g_param.ip_addr_src, 4);
	memcpy(&udp_pkt->ip.addr_dst.addr, &g_param.ip_addr_dst, 4);

	udp_pkt->udp_port_src = g_param.ip_port_src;
	udp_pkt->udp_port_dst = g_param.ip_port_dst;

	/* create a mutex for catching UDP reply */
	udp_mutex = CreateMutex(IOP_MUTEX_LOCKED);

	/* install a timer for UDP reply timeout */
	USec2SysClock(UDP_TIMEOUT, &timeout_sysclock);
	SetAlarm(&timeout_sysclock, timer_intr_handler, NULL);
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

	udp_pkt->ip.len = htons(pktsize - 14);				/* Subtract the ethernet header size */

	udp_pkt->ip.csum = 0;
	udp_pkt->ip.csum = inet_chksum(&udp_pkt->ip, 20);	/* Checksum the IP header (20 bytes) */

	udpsize = htons(size + 8);							/* Size of the UDP header + data */
	udp_pkt->udp_len = udpsize;
	memcpy(udp_sndbuf + sizeof(udp_pkt_t), buf, size);

	udp_pkt->udp_csum = 0;
	udp_pkt->udp_csum = inet_chksum_pseudo(&udp_sndbuf[34], &g_param.ip_addr_src, &g_param.ip_addr_dst, 8+size);

	/* send the ethernet frame */
	CpuSuspendIntr(&oldstate);
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
		if (udp_pkt->udp_port_dst == g_param.ip_port_src) {

			/* check pseudo checksum */
			if (inet_chksum_pseudo(&udp_pkt->udp_port_src, &g_param.ip_addr_dst, &g_param.ip_addr_src, (ntohs(udp_pkt->ip.len)-20)) == 0) {
				p_rcptbuf = buf;
				rcptbuf_size = size;
				iSignalSema(udp_mutex);
			}
		}
	}

	return 1;
}

/*
 * main UDP server thread
 */
void udp_server_thread(void *args)
{
	int oldstate;

	if (!adb_init(0)) {
		while (1) {
			WaitSema(udp_mutex);

			while (QueryIntrContext())
				DelayThread(10000);

			/* we got a valid UDP packet */
			CpuSuspendIntr(&oldstate);
			memcpy(udp_rcvbuf, p_rcptbuf, rcptbuf_size);
			CpuResumeIntr(oldstate);

			/* get packet data pointer and packet size */
			udp_pkt_t *udp_pkt = (udp_pkt_t *)udp_rcvbuf;
			int size = ntohs(udp_pkt->udp_len) - 8; /* substract udp header */
			char *data = (char *)&udp_rcvbuf[42];
			M_PRINTF("Got incoming UDP packet (%d bytes) at port %d: %s\n", size, ntohs(g_param.ip_port_src), data);

			/* output the received datas */
			udp_output(data, size);
			M_PRINTF("Forwarded packet (%d bytes) to port %d: %s\n", size, ntohs(g_param.ip_port_dst), data);
		}
	}

	ExitDeleteThread();
}
