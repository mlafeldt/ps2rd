/*
 * udp.c - lightweight UDP implementation
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
#include "ip.h"
#include "linux/if_ether.h"
#include "linux/ip.h"
#include "linux/udp.h"

#define M_PRINTF(format, args...) \
	printf(ADB_MODNAME ": " format, ## args)

typedef struct {
	/* Ethernet header (14) */
	struct ethhdr eth;

	/* IP header (20) */
	struct iphdr ip;

	/* UDP header (8) */
	struct udphdr udp;

	/* Data goes here */
} udp_pkt_t __attribute__((packed));

static u32 g_ip_addr_dst;
static u32 g_ip_addr_src;

static u8 udp_sndbuf[ETH_FRAME_LEN+8] __attribute__((aligned(64))); /* 1 MTU(1514) max */
static u8 *p_rcptbuf;
static int rcptbuf_size;
static int udp_rcv_mutex = -1;

#define UDP_MAX_ACTIVE_CONN	4
#define UDP_MAX_PASSIVE_CONN	2
#define UDP_MAX_CONN		(UDP_MAX_ACTIVE_CONN+UDP_MAX_PASSIVE_CONN)
#define UDP_LOCAL_PORT		IP_PORT(8341)

typedef struct {
	u16 status;
	u16 port;
	int mutex;
} udp_conn_t;

static udp_conn_t udp_conn_slots[UDP_MAX_ACTIVE_CONN+UDP_MAX_PASSIVE_CONN];

#if 0
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
		iSignalSema(udp_rcv_mutex);

	iSetAlarm(&timeout_sysclock, timer_intr_handler, NULL);

	return (unsigned int)args;
}
#endif

/*
 * udp_init: initialize UDP layer
 */
void udp_init(u32 ip_addr_dst, u32 ip_addr_src)
{
#if 0
	/* install a timer for UDP reply timeout */
	USec2SysClock(UDP_TIMEOUT, &timeout_sysclock);
	SetAlarm(&timeout_sysclock, timer_intr_handler, NULL);
#endif
	/* these are stored in network byte order, careful later */
	g_ip_addr_dst = ip_addr_dst;
	g_ip_addr_src = ip_addr_src;
}

/*
 * udp_output: send an UDP packet to IP layer
 */
int udp_output(u16 ip_port_src, u16 ip_port_dst, void *buf, int size)
{
	udp_pkt_t *udp_pkt;
	int r, oldstate;

	udp_pkt = (udp_pkt_t *)udp_sndbuf;

	udp_pkt->udp.source = ip_port_src;
	udp_pkt->udp.dest = ip_port_dst;
	udp_pkt->udp.len = htons(size + sizeof(struct udphdr));

	if (size > (ETH_DATA_LEN - sizeof(struct iphdr) - sizeof(struct udphdr)))
		size = (ETH_DATA_LEN - sizeof(struct iphdr) - sizeof(struct udphdr));

	memcpy(udp_sndbuf + sizeof(udp_pkt_t), buf, size);

	udp_pkt->udp.check = 0;
	udp_pkt->udp.check = inet_chksum_pseudo(&udp_sndbuf[ETH_HLEN+sizeof(struct iphdr)], &g_ip_addr_src, &g_ip_addr_dst, sizeof(struct udphdr) + size);

	CpuSuspendIntr(&oldstate);
	r = ip_output(udp_sndbuf, size + sizeof(struct udphdr), 0x11);
	CpuResumeIntr(oldstate);

	return r;
}

/*
 * udp_input: Called IP layer when a UDP packet is
 * received. (careful with Intr context)
 */
int udp_input(void *buf, int size)
{
	if (udp_rcv_mutex >= 0) {
		udp_pkt_t *udp_pkt = (udp_pkt_t *)buf;

		/* check pseudo checksum */
		if (inet_chksum_pseudo(&udp_pkt->udp, &g_ip_addr_dst, &g_ip_addr_src, (ntohs(udp_pkt->ip.tot_len)-sizeof(struct iphdr))) == 0) {
			p_rcptbuf = buf;
			rcptbuf_size = size;
			iSignalSema(udp_rcv_mutex);
		}
	}

	return 1;
}

/*
 * udp_open: connect/bind to a port
 */
int udp_open(int *s, u16 ip_port, int flags)
{
	int r, i, oldstate;
	int act_cnt = 0;
	int psv_cnt = 0;

	CpuSuspendIntr(&oldstate);

	for (i=0; i<UDP_MAX_CONN; i++) {
		if (udp_conn_slots[i].port == ip_port) {
			r = -1; /* already connected/bound */
			goto out;
		}
		if (udp_conn_slots[i].status == UDP_ACTIVE_CONN)
			act_cnt++;
		else if (udp_conn_slots[i].status == UDP_PASSIVE_CONN)
			psv_cnt++;
	}

	if ((flags == UDP_ACTIVE_CONN) && (act_cnt == UDP_MAX_ACTIVE_CONN)) {
		r = -2;
		goto out;
	}
	else if ((flags == UDP_PASSIVE_CONN) && (psv_cnt == UDP_MAX_PASSIVE_CONN)) {
		r = -2;
		goto out;
	}

	for (i=0; i<UDP_MAX_CONN; i++) {
		if (udp_conn_slots[i].status == 0) {
			if (flags == UDP_ACTIVE_CONN)
				udp_conn_slots[i].mutex = CreateMutex(IOP_MUTEX_UNLOCKED);
			else if (flags == UDP_PASSIVE_CONN)
				udp_conn_slots[i].mutex = CreateMutex(IOP_MUTEX_LOCKED);
			udp_conn_slots[i].port = ip_port;
			udp_conn_slots[i].status = flags;
			*s = i;
			r = 0;
			goto out;
		}
	}

	r = -3; /* no more slots free */

out:
	CpuResumeIntr(oldstate);

	return r;
}

/*
 * udp_close: close connection/stop listening for a port
 */
int udp_close(int s)
{
	int r, oldstate;

	CpuSuspendIntr(&oldstate);

	if (udp_conn_slots[s].status != 0) {
		DeleteSema(udp_conn_slots[s].mutex);
		udp_conn_slots[s].port = 0;
		udp_conn_slots[s].status = 0;
		r = 0;
		goto out;
	}
	else
		r = -1;

out:
	CpuResumeIntr(oldstate);

	return r;
}

/*
 * udp_recv: receive datas trought a passive udp connection
 */
int udp_recv(int s, void *buf, int size)
{
	int r, oldstate;
	register int pktsize;

	CpuSuspendIntr(&oldstate);
	if (udp_conn_slots[s].status != UDP_PASSIVE_CONN) {
		r = -1;
		goto out;
	}
	udp_rcv_mutex = udp_conn_slots[s].mutex;
	CpuResumeIntr(oldstate);

wait:
	WaitSema(udp_conn_slots[s].mutex);

#if 0
	while (QueryIntrContext())
		DelayThread(10000);
#endif

	CpuSuspendIntr(&oldstate);

	/* get packet data pointer and packet size */
	udp_pkt_t *udp_pkt = (udp_pkt_t *)p_rcptbuf;

	/* does it match our bound port ? */
	if (udp_pkt->udp.dest == udp_conn_slots[s].port) {

		pktsize = ntohs(udp_pkt->udp.len) - sizeof(struct udphdr); /* substract udp header */
		if (pktsize > size)
			pktsize = size;

		//p_rcptbuf[ETH_HLEN+sizeof(struct iphdr)+8+pktsize] = 0;
		memcpy(buf, &p_rcptbuf[ETH_HLEN+sizeof(struct iphdr)+sizeof(struct udphdr)], pktsize);
		r = pktsize;
	}
	else
		goto wait;

out:
	udp_rcv_mutex = -1;
	CpuResumeIntr(oldstate);

	return r;
}

/*
 * udp_send: send data over an active UDP connection
 */
int udp_send(int s, void *buf, int size)
{
	int r, oldstate;

	CpuSuspendIntr(&oldstate);
	if (udp_conn_slots[s].status != UDP_ACTIVE_CONN) {
		r = -1;
		goto out;
	}

	WaitSema(udp_conn_slots[s].mutex);
	r = udp_output(UDP_LOCAL_PORT, udp_conn_slots[s].port, buf, size);
	SignalSema(udp_conn_slots[s].mutex);

out:
	CpuResumeIntr(oldstate);

	return r;
}
