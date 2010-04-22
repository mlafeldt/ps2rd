.SILENT:

IOP_MODULES = debugger dev9 eesync memdisk usb_mass smap

ifeq ($(SMS_MODULES),1)
IOP_MODULES += SMSMAP SMSTCPIP
endif

all:
	@for m in $(IOP_MODULES); do \
		echo "* [IOP] Building $$m ..."; \
		$(MAKE) -C $$m; \
	done

clean:
	@for m in $(IOP_MODULES); do \
		echo "* [IOP] Cleaning $$m ..."; \
		$(MAKE) -C $$m clean; \
	done

rebuild: clean all
