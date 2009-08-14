##
# Makefile for boot loader
#
# Copyright (C) 2009 misfire <misfire@xploderfreax.de>
##

ifeq ($(DEBUG),1)
EE_CFLAGS += -D_DEBUG -g
EE_LDFLAGS += -g
else
EE_LDFLAGS += -s
endif

EE_INCS += -I$(PS2SDK)/ports/include
EE_CFLAGS += #-D_NO_HOOK -D_NO_SBV -D_NO_IOPRESET
EE_LDFLAGS += -L$(PS2SDK)/ports/lib
EE_ASFLAGS +=

EE_BIN = artemis.elf

ERL_FILES = engine_erl.o libkernel_erl.o debugger_erl.o elfldr_erl.o
ifeq ($(DEBUG),1)
ERL_FILES += libc_erl.o libdebug_erl.o libpatches_erl.o
endif

IRX_FILES = ps2dev9_irx.o ps2ip_irx.o ps2smap_irx.o debugger_irx.o netlog_irx.o
LDR_FILES = loader.o configman.o erlman.o engineman.o mycdvd.o mypad.o \
	mystring.o myutil.o netlog_rpc.o

EE_OBJS = $(LDR_FILES) $(ERL_FILES) $(IRX_FILES)

EE_LIBS = -lcdvd -ldebug -lpad -lpatches -lerl -lconfig -lcheats


all: loader

clean:
	rm -f $(EE_BIN) $(LDR_FILES)

rebuild: clean all

loader: $(EE_OBJS) $(PS2SDK)/ee/startup/crt0.o
	$(EE_CC) -mno-crt0 -T$(PS2SDK)/ee/startup/linkfile $(EE_CFLAGS) \
	-o $(EE_BIN) $(PS2SDK)/ee/startup/crt0.o $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIBS)

pack: all
	ps2-packer $(EE_BIN) $(EE_BIN).2 -v
	mv $(EE_BIN).2 $(EE_BIN)
	chmod +x $(EE_BIN)

run:
	ps2client reset; sleep 3
	ps2client execee host:$(EE_BIN)


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
