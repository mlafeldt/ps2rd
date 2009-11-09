.SILENT:

IOP_MODULES = debugger dev9 eesync memdisk usb_mass

ifeq ($(SMS_MODULES),1)
IOP_MODULES += SMSMAP SMSTCPIP
else
IOP_MODULES += smap
endif

all:
	@for m in $(IOP_MODULES); do \
		echo "* [IOP] Building $$m ..."; \
		make -C $$m; \
	done

clean:
	@for m in $(IOP_MODULES); do \
		echo "* [IOP] Cleaning $$m ..."; \
		make -C $$m clean; \
	done

rebuild: clean all
