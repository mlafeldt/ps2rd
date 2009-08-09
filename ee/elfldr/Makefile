##
# Makefile for ELF loader
##

EE_INCS +=
EE_CFLAGS +=
EE_LDFLAGS += -nostartfiles -s
EE_LIBS =

EE_BIN = elfldr.elf
EE_OBJS = elfldr.o


all: $(EE_OBJS) $(PS2SDK)/ee/startup/crt0.o
	$(EE_CC) -mno-crt0 -Tlinkfile $(EE_CFLAGS) -o $(EE_BIN) \
	$(PS2SDK)/ee/startup/crt0.o $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIBS)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

rebuild: clean all


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
