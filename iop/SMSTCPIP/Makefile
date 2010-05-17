IOP_BIN = SMSTCPIP.irx
IOP_OBJS = ps2ip.o inet.o ip.o ip_addr.o ip_frag.o etharp.o tcp_in.o tcp_out.o \
	tcp.o tcpip.o mem.o api_lib.o api_msg.o sockets.o netif.o udp.o memp.o \
	icmp.o pbuf.o exports.o imports.o
IOP_LIBS =

IOP_INCS +=
IOP_CFLAGS += -Wall -fno-builtin-printf -DLWIP_NOASSERT -DLOW_MEM_FOOTPRINT
IOP_ASFLAGS +=
IOP_LDFLAGS += -s

ifeq ($(NETLOG),1)
IOP_CFLAGS += -D_NETLOG
endif

all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

run:
	ps2client -t 1 execiop host:$(IOP_BIN)


include $(PS2SDK)/Defs.make
include Rules.make
