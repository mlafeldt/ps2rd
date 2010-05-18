##
# Makefile for boot loader
#
# Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
##

ifeq ($(DEBUG),1)
EE_CFLAGS += -D_DEBUG -g
EE_LDFLAGS += -g
else
EE_LDFLAGS += -s
endif

EE_INCS += -I$(LIBCHEATS)/include -I$(LIBCONFIG)/include
EE_CFLAGS +=
EE_LDFLAGS += -L$(LIBCHEATS)/lib -L$(LIBCONFIG)/lib
EE_ASFLAGS +=

EE_BIN = ps2rd.elf

ERL_FILES = engine_erl.o libkernel_erl.o libpatches_erl.o \
	debugger_erl.o elfldr_erl.o videomod_erl.o
ifeq ($(DEBUG),1)
ERL_FILES +=
endif

IRX_FILES = ps2dev9_irx.o ps2ip_irx.o ps2smap_irx.o \
	debugger_irx.o memdisk_irx.o eesync_irx.o \
	usbd_irx.o usb_mass_irx.o
ifeq ($(SMS_MODULES),1)
EE_CFLAGS += -D_SMS_MODULES
IRX_FILES += ps2ip_sms_irx.o ps2smap_sms_irx.o
endif

LDR_FILES = loader.o configman.o erlman.o irxman.o gameid.o

EE_OBJS = $(LDR_FILES) $(ERL_FILES) $(IRX_FILES)

EE_LIBS = -lcdvd -ldebug -lpad -lpatches -lerl -lconfig -lcheats


all: loader

clean:
	rm -f $(EE_BIN) $(LDR_FILES)

loader: $(EE_OBJS) $(PS2SDK)/ee/startup/crt0.o
	$(EE_CC) -mno-crt0 -T$(PS2SDK)/ee/startup/linkfile $(EE_CFLAGS) \
	-o $(EE_BIN) $(PS2SDK)/ee/startup/crt0.o $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIBS)

pack:
	ps2-packer $(EE_BIN) $(EE_BIN).2 -v
	mv $(EE_BIN).2 $(EE_BIN)
	chmod +x $(EE_BIN)

run:
	ps2client reset; sleep 3
	ps2client execee host:$(EE_BIN)


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
