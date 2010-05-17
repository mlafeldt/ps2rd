IOP_BIN = skeleton.irx
IOP_OBJS = skeleton.o exports.o imports.o
IOP_LIBS =

IOP_INCS +=
IOP_CFLAGS += -Wall -fno-builtin -DSKELETON_RPC
IOP_ASFLAGS +=
IOP_LDFLAGS += -s


all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

run:
	ps2client -t 1 execiop host:$(IOP_BIN)


include $(PS2SDK)/Defs.make
include Rules.make
