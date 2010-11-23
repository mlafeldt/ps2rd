EE_INCS +=
EE_CFLAGS += -D_HOOK_9
EE_CFLAGS += -DMAX_HOOKS=5
EE_CFLAGS += -DMAX_CODES=100
EE_CFLAGS += -DMAX_CALLBACKS=1
EE_CXXFLAGS +=
EE_LDFLAGS +=
EE_ASFLAGS +=
EE_LIBS +=

EE_OBJS += engine_asm.S
EE_OBJS += engine.o

TEST_OBJS += engine_asm.S
TEST_OBJS += test.o
TEST_OBJS += $(PS2SDK)/ee/startup/crt0.o

all: engine.erl

clean:
	rm -f *.o *.elf *.erl

engine.erl: $(EE_OBJS)
	$(EE_CC) -mno-crt0 -nostartfiles -o $@ $(EE_OBJS) $(EE_CFLAGS) \
		$(EE_INCS) $(EE_LDFLAGS) -Wl,-r -Wl,-d
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $@

test.elf: $(TEST_OBJS)
	$(EE_CC) -mno-crt0 -T$(PS2SDK)/ee/startup/linkfile $(EE_CFLAGS) $(EE_INCS) \
	-o $@ $(TEST_OBJS) $(EE_LDFLAGS) $(EE_LIBS)

test: test.elf
	ps2client reset; sleep 3
	ps2client execee host:test.elf


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
