IOP_MODULES=dev9 smap debugger memdisk eesync usb_mass

all:
	@for m in $(IOP_MODULES); do \
		make -C $$m; \
	done

clean:
	@for m in $(IOP_MODULES); do \
		make -C $$m clean; \
	done

rebuild: clean all
