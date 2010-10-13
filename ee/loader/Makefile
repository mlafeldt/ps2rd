##
# Makefile for boot loader
#
# Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
#
# $Id$
##

EE_INCS += -I$(LIBCHEATS)/include -I$(LIBCONFIG)/include
EE_CFLAGS += -DPS2RD_VERSION='"$(PS2RD_VERSION)"'
EE_LDFLAGS += -L$(LIBCHEATS)/lib -L$(LIBCONFIG)/lib
EE_ASFLAGS +=

EE_BIN = ps2rd.elf

EE_OBJS += debugger_erl.o
EE_OBJS += elfldr_erl.o
EE_OBJS += engine_erl.o
EE_OBJS += libkernel_erl.o
EE_OBJS += libpatches_erl.o
EE_OBJS += videomod_erl.o

EE_OBJS += debugger_irx.o
EE_OBJS += eesync_irx.o
EE_OBJS += memdisk_irx.o
EE_OBJS += ps2dev9_irx.o
EE_OBJS += ps2ip_irx.o
EE_OBJS += ps2smap_irx.o
EE_OBJS += usb_mass_irx.o
EE_OBJS += usbd_irx.o

EE_OBJS += cheatman.o
EE_OBJS += configman.o
EE_OBJS += elfid.o
EE_OBJS += erlman.o
EE_OBJS += irxman.o
EE_OBJS += loader.o

EE_LIBS += -lcdvd
EE_LIBS += -lcheats
EE_LIBS += -lconfig
EE_LIBS += -ldebug
EE_LIBS += -lerl
EE_LIBS += -lpad
EE_LIBS += -lpatches

ifeq ($(DEBUG),1)
EE_CFLAGS += -D_DEBUG -g
EE_LDFLAGS += -g
else
EE_LDFLAGS += -s
endif

ifndef NO_SMS
EE_CFLAGS += -D_SMS_MODULES
EE_OBJS += ps2ip_sms_irx.o ps2smap_sms_irx.o
endif


all: loader

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

loader: $(EE_OBJS) $(PS2SDK)/ee/startup/crt0.o
	$(EE_CC) -mno-crt0 -T$(PS2SDK)/ee/startup/linkfile $(EE_CFLAGS) \
	-o $(EE_BIN) $(PS2SDK)/ee/startup/crt0.o $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIBS)

loader.o: mycdvd.h mypad.h myutil.h dbgprintf.h ../../PS2RD-VERSION-FILE

pack:
	ps2-packer $(EE_BIN) $(EE_BIN).2 -v
	mv $(EE_BIN).2 $(EE_BIN)
	chmod +x $(EE_BIN)

run:
	ps2client reset; sleep 3
	ps2client execee host:$(EE_BIN)


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
