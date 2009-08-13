##
# Makefile for ELF loader
##

EE_INCS +=
EE_CFLAGS +=
EE_CXXFLAGS +=
EE_LDFLAGS +=
EE_ASFLAGS +=
EE_LIBS +=
EE_OBJS = elfldr.o syscall_hooks.S


all: elfldr.erl

clean:
	rm -f *.o *.erl

rebuild: clean all

elfldr.erl: $(EE_OBJS)
	$(EE_CC) -mno-crt0 -nostartfiles -o $@ $(EE_OBJS) $(EE_CFLAGS) \
		$(EE_INCS) $(EE_LDFLAGS) -Wl,-r
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $@


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
