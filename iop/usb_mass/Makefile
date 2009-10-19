IOP_BIN = usb_mass.irx
IOP_OBJS = mass_stor.o fat_driver.o scache.o usb_mass.o imports.o
IOP_LDFLAGS += -Wl,-s
IOP_CFLAGS += -D_PS2_ -I$(PS2SDKSRC)/iop/usb/usbd/include/

#uncomment to enable write support
WRITE_SUPPORT=1

#uncomment to compile in dump/info functions
#COMPILE_DUMPS=1

ifdef WRITE_SUPPORT
IOP_CFLAGS += -DWRITE_SUPPORT
IOP_OBJS += fat_write.o
endif

ifdef COMPILE_DUMPS
IOP_CFLAGS += -DCOMPILE_DUMPS
endif

all: $(IOP_BIN)

clean:
	rm -f *.elf *.o *.a *.irx



include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release


# A rule to build imports.lst.
%.o : %.lst
	echo "#include \"irx_imports.h\"" > build-imports.c
	cat $< >> build-imports.c
	$(IOP_CC) $(IOP_CFLAGS) -c build-imports.c -o $@
	-rm -f build-imports.c
