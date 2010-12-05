EE_INCS +=
EE_CFLAGS +=
EE_CXXFLAGS +=
EE_LDFLAGS +=
EE_ASFLAGS +=
EE_LIBS +=

EE_OBJS += debugger.o
EE_OBJS += erl.o
EE_OBJS += loadfile.o
EE_OBJS += loadmodule_hooks.o
EE_OBJS += ntpb.o
EE_OBJS += padread_hooks.o
EE_OBJS += pattern.o
EE_OBJS += syscall_hooks.S

ifeq ($(DEBUG),1)
EE_CFLAGS += -D_DEBUG
endif

all: debugger.erl

clean:
	rm -f *.o *.erl *.erl

debugger.erl: $(EE_OBJS)
	$(EE_CC) -mno-crt0 -nostartfiles -o $@ $(EE_OBJS) $(EE_CFLAGS) \
		$(EE_INCS) $(EE_LDFLAGS) -Wl,-r -Wl,-d
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $@


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
