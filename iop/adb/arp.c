/*
 * arp.c - Advanced debugger
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

#include "arp.h"
#include "inet.h"
#include "smap.h"
#include "linux/if_ether.h"

#define ARP_TIMEOUT	(3000*1000)
#define ARP_REQUEST	0x01
#define ARP_REPLY	0x02

typedef struct {
	/* Ethernet header (14) */
	struct ethhdr eth;

	/* ARP header (28) */
	u16	arp_hwtype;
	u16	arp_protocoltype;
	u8	arp_hwsize;
	u8	arp_protocolsize;
	u16	arp_opcode;
	u8	arp_sender_eth_addr[ETH_ALEN];
	ip_addr_t arp_sender_ip_addr;
	u8	arp_target_eth_addr[ETH_ALEN];
	ip_addr_t arp_target_ip_addr;
} arp_pkt_t __attribute__((packed));

static u8 g_eth_addr_dst[ETH_ALEN];
static u8 g_eth_addr_src[ETH_ALEN];
static u32 g_ip_addr_src;
static u32 g_ip_addr_dst;

static int arp_mutex = -1;
static int arp_reply_flag;
static int wait_arp_reply = 0;

/*
 * arp_init: initialize ARP layer
 */
void arp_init(u32 dst_ip, u32 src_ip)
{
	/* these are stored in network byte order, careful later */
	memset(g_eth_addr_dst, 0xff, ETH_ALEN);
	memset(g_eth_addr_src, 0xff, ETH_ALEN);
	g_ip_addr_dst = dst_ip;
	g_ip_addr_src = src_ip;
}

/*
 * arp_output: send an ARP ethernet frame
 */
static void arp_output(u16 opcode, u8 *target_eth_addr)
{
	arp_pkt_t arp_pkt;

	memset(&arp_pkt, 0, sizeof(arp_pkt_t));
	memcpy(arp_pkt.eth.h_dest, g_eth_addr_dst, ETH_ALEN);
	memcpy(arp_pkt.eth.h_source, g_eth_addr_src, ETH_ALEN);
	arp_pkt.eth.h_proto = HTONS(ETH_P_ARP);

	arp_pkt.arp_hwtype = HTONS(ETH_P_802_3);
	arp_pkt.arp_protocoltype = HTONS(ETH_P_IP);
	arp_pkt.arp_hwsize = ETH_ALEN;		/* size of HW MAC adresses */
	arp_pkt.arp_protocolsize = 4;
	arp_pkt.arp_opcode = htons(opcode);
	memcpy(arp_pkt.arp_sender_eth_addr, g_eth_addr_src, ETH_ALEN);
	memcpy(&arp_pkt.arp_sender_ip_addr, &g_ip_addr_src, 4);
	memcpy(arp_pkt.arp_target_eth_addr, target_eth_addr, ETH_ALEN);
	memcpy(&arp_pkt.arp_target_ip_addr, &g_ip_addr_dst, 4);

	while (smap_xmit(&arp_pkt, sizeof(arp_pkt_t)) != 0)
		;
}

/*
 * arp_input: Called from smap RX intr handler when an ARP ethernet
 * frame is received. (careful with Intr context)
 */
void arp_input(void *buf, int size)
{
	register int i;
	arp_pkt_t *arp_pkt = (arp_pkt_t *)buf;

	/* check if it's an ARP reply */
	if (arp_pkt->arp_opcode == NTOHS(ARP_REPLY)) {

		if (wait_arp_reply) {
			memcpy(g_eth_addr_dst, &arp_pkt->arp_sender_eth_addr[0], ETH_ALEN);

			arp_reply_flag = 1;
			iSignalSema(arp_mutex);
		}
	}
	/* check if it's an ARP request */
	else if (arp_pkt->arp_opcode == NTOHS(ARP_REQUEST)) {

		/* Is that request for us ? */
		for (i=0; i<4; i++) {
			u8 *p = (u8 *)&g_ip_addr_src;
			if (arp_pkt->arp_target_ip_addr.addr[i] != p[i])
				break;
		}

		/* yes ? we send an ARP reply with our ethernet addr */
		if (i == 4)
			arp_output(ARP_REPLY, g_eth_addr_dst);
	}
}

/*
 * Timer Interrupt handler for ARP reply timeout (Intr context)
 */
static unsigned int timer_intr_handler(void *args)
{
	iSignalSema(arp_mutex);

	return (unsigned int)args;
}

/*
 * arp_request: send an ARP request, should be called 1st
 */
void arp_request(u8 *eth_addr)
{
	iop_sys_clock_t sysclock;
	int oldstate;

	/* create a mutex for catching ARP reply */
	arp_mutex = CreateMutex(IOP_MUTEX_LOCKED);

	CpuSuspendIntr(&oldstate);
	wait_arp_reply = 1;

send_arp_request:

	/* send an ARP resquest */
	arp_output(ARP_REQUEST, "\x00\x00\x00\x00\x00\x00");
	CpuResumeIntr(oldstate);

	/* set the reply timer */
	USec2SysClock(ARP_TIMEOUT, &sysclock);
	SetAlarm(&sysclock, timer_intr_handler, NULL);

	/* Wait for ARP reply or timeout */
	WaitSema(arp_mutex);

	/*while (QueryIntrContext())
		DelayThread(10000);*/

	CpuSuspendIntr(&oldstate);
	CancelAlarm(timer_intr_handler, NULL);

	if (arp_reply_flag == 0) 	/* It was a timeout ? */
		goto send_arp_request; 	/* yes, so retry...   */

	wait_arp_reply = 0;
	arp_reply_flag = 0;
	memcpy(eth_addr, g_eth_addr_dst, ETH_ALEN);
	CpuResumeIntr(oldstate);

	/* delete the mutex */
	DeleteSema(arp_mutex);
	arp_mutex = -1;
}
