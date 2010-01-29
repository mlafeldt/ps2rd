#ifndef _IOP_IP_H_
#define _IOP_IP_H_

#include <tamtypes.h>

/* These automatically convert the address and port to network order.  */
#define IP_ADDR(a, b, c, d)	(((d & 0xff) << 24) | ((c & 0xff) << 16) | \
				((b & 0xff) << 8) | ((a & 0xff)))
#define IP_PORT(port)	(((port & 0xff00) >> 8) | ((port & 0xff) << 8))


void ip_input(void *buf, int size);

#endif /* _IOP_IP_H_ */
