IOP_BIN  = usb_mass.irx
IOP_OBJS = mass_stor.o fat_driver.o fat_write.o scache.o usb_mass.o imports.o
IOP_LIBS =

IOP_INCS +=
IOP_CFLAGS  += -fno-builtin -D_PS2_ -DWRITE_SUPPORT
IOP_LDFLAGS += -s


all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

rebuild: clean all

run:
	ps2client -t 1 execiop host:$(IOP_BIN)


include $(PS2SDK)/Defs.make
include Rules.make
