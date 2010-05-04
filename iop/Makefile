.SILENT:

IOP_MODULES = debugger dev9 eesync memdisk usb_mass smap

ifeq ($(SMS_MODULES),1)
IOP_MODULES += SMSMAP SMSTCPIP
endif

all:
	@for i in $(IOP_MODULES); do \
		echo "* [IOP] Building irx $$i ..."; \
		$(MAKE) -C $$i; \
	done

clean:
	@for i in $(IOP_MODULES); do \
		echo "* [IOP] Cleaning $$i ..."; \
		$(MAKE) -C $$i clean; \
	done

rebuild: clean all
