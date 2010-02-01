.SILENT:

IOP_MODULES = adb dev9 eesync memdisk net usb_mass #debugger

#ifeq ($(SMS_MODULES),1)
#IOP_MODULES += SMSMAP SMSTCPIP
#else
#IOP_MODULES += smap
#endif

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
