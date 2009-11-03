IOP_MODULES=debugger dev9 eesync memdisk usb_mass SMSMAP SMSTCPIP

all:
	@for m in $(IOP_MODULES); do \
		make -C $$m; \
	done

clean:
	@for m in $(IOP_MODULES); do \
		make -C $$m clean; \
	done

rebuild: clean all
