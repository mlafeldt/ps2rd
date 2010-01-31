/*
 * arp.c - lightweight ARP implementation
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
#include <intrman.h>
#include <thbase.h>
#include <thsemap.h>
#include <sysclib.h>

#include "arp.h"
#include "inet.h"
#include "eth.h"
#include "linux/if_ether.h"
#include "linux/if_arp.h"

#define ARP_TIMEOUT	(3000*1000)

struct arpdata {
	u8 ar_sha[ETH_ALEN];       /* sender hardware address      */
	u8 ar_sip[4];              /* sender IP address            */
	u8 ar_tha[ETH_ALEN];       /* target hardware address      */
	u8 ar_tip[4];              /* target IP address            */	
};

typedef struct {
	/* Ethernet header (14) */
	struct ethhdr eth;

	/* ARP header (8) */
	struct arphdr arp_hdr;

	/* ARP data (20) */
	struct arpdata arp;
} arp_pkt_t __attribute__((packed));

static u8 g_eth_addr_src[ETH_ALEN];
static u8 g_eth_addr_dst[ETH_ALEN];
static u32 g_ip_addr_dst;
static u32 g_ip_addr_src;

static int arp_mutex = -1;
static int arp_reply_flag;
static int wait_arp_reply = 0;

/*
 * arp_init: initialize ARP layer
 */
void arp_init(u8 *eth_addr_dst, u8 *eth_addr_src, u32 dst_ip, u32 src_ip)
{
	/* these are stored in network byte order, careful later */
	memset(g_eth_addr_dst, 0, ETH_ALEN);	
	memcpy(g_eth_addr_src, eth_addr_src, ETH_ALEN);	
	g_ip_addr_dst = dst_ip;
	g_ip_addr_src = src_ip;

	/* Does ARP request to get PC ethernet addr */
	arp_request(eth_addr_dst);
}

/*
 * arp_output: send an ARP ethernet frame
 */
static int arp_output(u16 opcode, const u8 *target_eth_addr)
{
	arp_pkt_t arp_pkt;

	memset(&arp_pkt, 0, sizeof(arp_pkt_t));

	arp_pkt.arp_hdr.ar_hrd = HTONS(ETH_P_802_3);
	arp_pkt.arp_hdr.ar_pro = HTONS(ETH_P_IP);
	arp_pkt.arp_hdr.ar_hln = ETH_ALEN;
	arp_pkt.arp_hdr.ar_pln = 4;
	arp_pkt.arp_hdr.ar_op = htons(opcode);

	memcpy(arp_pkt.arp.ar_sha, g_eth_addr_src, ETH_ALEN);
	memcpy(&arp_pkt.arp.ar_sip, &g_ip_addr_src, 4);
	memcpy(arp_pkt.arp.ar_tha, target_eth_addr, ETH_ALEN);
	memcpy(&arp_pkt.arp.ar_tip, &g_ip_addr_dst, 4);

	return eth_output(&arp_pkt, sizeof(arp_pkt_t), HTONS(ETH_P_ARP));
}

/*
 * arp_input: Called from ethernet layer when an ARP ethernet
 * frame is received. (careful with Intr context)
 */
void arp_input(void *buf, int size)
{
	arp_pkt_t *arp_pkt = (arp_pkt_t *)buf;

	if (arp_pkt->arp_hdr.ar_op == NTOHS(ARPOP_REPLY)) { /* process ARP reply */
		if (wait_arp_reply) {
			memcpy(g_eth_addr_dst, &arp_pkt->arp.ar_sha, ETH_ALEN);
			arp_reply_flag = 1;
			iSignalSema(arp_mutex);
		}
	} else if (arp_pkt->arp_hdr.ar_op == NTOHS(ARPOP_REQUEST)) { /* process ARP request */
		/* if request is for us, reply to the sender with our ethernet addr */
		if (!memcmp(arp_pkt->arp.ar_tip, &g_ip_addr_src, 4))
			arp_output(ARPOP_REPLY, arp_pkt->arp.ar_sha);
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
	/* send an ARP request */
	arp_output(ARPOP_REQUEST, "\x00\x00\x00\x00\x00\x00");
	CpuResumeIntr(oldstate);

	/* set the reply timer */
	USec2SysClock(ARP_TIMEOUT, &sysclock);
	SetAlarm(&sysclock, timer_intr_handler, NULL);

	/* wait for ARP reply or timeout */
	WaitSema(arp_mutex);
#if 0
	while (QueryIntrContext())
		DelayThread(10000);
#endif
	CpuSuspendIntr(&oldstate);
	CancelAlarm(timer_intr_handler, NULL);

	/* retry on timeout */
	if (arp_reply_flag == 0)
		goto send_arp_request;

	wait_arp_reply = 0;
	arp_reply_flag = 0;
	memcpy(eth_addr, g_eth_addr_dst, ETH_ALEN);
	CpuResumeIntr(oldstate);

	/* delete the mutex */
	DeleteSema(arp_mutex);
	arp_mutex = -1;
}
