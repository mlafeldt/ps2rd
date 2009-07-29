##
# Makefile for debugger
#
# Copyright (C) 2009 misfire <misfire@xploderfreax.de>
##

EE_INCS +=
EE_CFLAGS +=
EE_CXXFLAGS +=
EE_LDFLAGS +=
EE_ASFLAGS +=
EE_LIBS +=
EE_OBJS = debugger.c
TEST_OBJS = $(EE_OBJS) test.o $(PS2SDK)/ee/startup/crt0.o

all: debugger.erl test.elf

clean:
	rm -f *.erl *.rel *.o *.elf

rebuild: clean all

debugger.erl: $(EE_OBJS)
	$(EE_CC) -mno-crt0 -nostartfiles -o $@ $(EE_OBJS) $(EE_CFLAGS) $(EE_INCS) $(EE_LDFLAGS) -Wl,-r
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $@

test.elf: $(TEST_OBJS)
	$(EE_CC) -mno-crt0 -T$(PS2SDK)/ee/startup/linkfile $(EE_CFLAGS) $(EE_INCS) \
	-o $@ $(TEST_OBJS) $(EE_LDFLAGS) $(EE_LIBS)

run:
	ps2client reset; sleep 3
	ps2client execee host:test.elf


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
