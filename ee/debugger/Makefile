##
# Makefile for debugger
#
# Copyright (C) 2009 misfire <misfire@xploderfreax.de>
##

ifeq ($(DEBUG),1)
EE_CFLAGS += -D_DEBUG
endif

EE_INCS +=
EE_CFLAGS +=
EE_CXXFLAGS +=
EE_LDFLAGS +=
EE_ASFLAGS +=
EE_LIBS +=
EE_OBJS = debugger.o debugger_rpc.o padread_hooks.o syscall_hooks.S


all: debugger.erl

clean:
	rm -f *.o debugger.erl

rebuild: clean all

debugger.erl: $(EE_OBJS)
	$(EE_CC) -mno-crt0 -nostartfiles -o $@ $(EE_OBJS) $(EE_CFLAGS) \
		$(EE_INCS) $(EE_LDFLAGS) -Wl,-r
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $@


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
