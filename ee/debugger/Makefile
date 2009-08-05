##
# Makefile for debugger
#
# Copyright (C) 2009 misfire <misfire@xploderfreax.de>
##

KRNL_SYM = -DF___syscall -DF_SifSetDma -DF_SifSetReg -DF_SetSyscall \
	-DF_GetSyscall -DF_DIntr -DF_EIntr

KRNL_OBJS = libkernel/kernel.S libkernel/getsyscall.o libkernel/glue.o

EE_INCS +=
EE_CFLAGS += $(KRNL_SYM)
EE_CXXFLAGS +=
EE_LDFLAGS +=
EE_ASFLAGS +=
EE_LIBS +=
EE_OBJS = debugger.o $(KRNL_OBJS)


all: debugger.erl

clean:
	rm -f *.o *.elf *.erl libkernel/*.o

rebuild: clean all

debugger.erl: $(EE_OBJS)
	$(EE_CC) -mno-crt0 -nostartfiles -o $@ $(EE_OBJS) $(EE_CFLAGS) $(EE_INCS) $(EE_LDFLAGS) -Wl,-r
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $@

run:
	ps2client reset; sleep 3
	ps2client execee host:test.elf


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
