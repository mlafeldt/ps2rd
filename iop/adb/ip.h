#ifndef _IOP_IP_H_
#define _IOP_IP_H_

#include <tamtypes.h>

/* These automatically convert the address and port to network order.  */
#define IP_ADDR(a, b, c, d)	(((d & 0xff) << 24) | ((c & 0xff) << 16) | \
				((b & 0xff) << 8) | ((a & 0xff)))
#define IP_PORT(port)	(((port & 0xff00) >> 8) | ((port & 0xff) << 8))

typedef struct { u8 addr[4]; } ip_addr_t __attribute__((packed));

/* IP header (20) */
typedef struct {
	u8	hlen;
	u8	tos;
	u16	len;
	u16	id;
	u8	flags;
	u8	frag_offset;
	u8	ttl;
	u8	proto;
	u16	csum;
	ip_addr_t addr_src;
	ip_addr_t addr_dst;
} ip_hdr_t;

#endif /* _IOP_IP_H_ */
