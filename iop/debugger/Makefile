#  _____     ___ ____
#   ____|   |    ____|
#  |     ___|   |____
#
#  ------------------------------------------------------------------------

IOP_BIN = debugger.irx
IOP_OBJS = ntpbserver.o imports.o

ifeq ($(NETLOG),1)
IOP_OBJS += netlog.o udpcl.o xprintf.o
IOP_CFLAGS += -D_NETLOG
endif

IOP_INCS += -I$(PS2SDK)/iop/include
IOP_CFLAGS += -Wall -fno-builtin $(DEBUG_FLAGS)
IOP_LDFLAGS += -s

all: $(IOP_BIN)

clean:
	-rm -f *.o *.irx

include $(PS2SDK)/Defs.make
include Rules.make

