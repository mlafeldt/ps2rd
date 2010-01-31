IOP_BIN = adb.irx
IOP_OBJS = adb.o smap.o eth.o arp.o inet.o ip.o udp.o exports.o imports.o
IOP_OBJS += tty.o # should be configureable
IOP_LIBS =

IOP_INCS +=
IOP_CFLAGS += -Wall -fno-builtin -DUDPTTY #-DADB_RPC
IOP_ASFLAGS +=
IOP_LDFLAGS += -s


all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

rebuild: clean all

run:
	ps2client reset && sleep 3
	ps2client execiop host:$(IOP_BIN)


include $(PS2SDK)/Defs.make
include Rules.make
