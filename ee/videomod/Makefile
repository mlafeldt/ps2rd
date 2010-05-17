##
# Makefile for videomod
#
# Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
##

EE_INCS +=
EE_CFLAGS +=
EE_CXXFLAGS +=
EE_LDFLAGS +=
EE_ASFLAGS +=
EE_LIBS +=
EE_OBJS = videomod_asm.S videomod.o

all: videomod.erl

clean:
	rm -f *.o *.elf *.erl

videomod.erl: $(EE_OBJS)
	$(EE_CC) -mno-crt0 -nostartfiles -o $@ $(EE_OBJS) $(EE_CFLAGS) \
		$(EE_INCS) $(EE_LDFLAGS) -Wl,-r -Wl,-d
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $@


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
