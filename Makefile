# This is the main Makefile of Artemis.
#
# The build targets are:
#  check   - check for environment variables (invoked by all)
#  all     - compile project (default)
#  clean   - clean project
#  rebuild - rebuild project (clean + all)
#  release - create "release package"
#
# The following variables can influence the build process:

# Set DEBUG to 1 to enable the debug mode. In debug mode, a lot of helpful debug
# messages will be printed to host: when using ps2link. There're also some other
# consequences mainly important for the developers.
DEBUG = 1

# When USB is 1, USB support will be activated by loading the IOP modules
# usbd.irx and usb_mass.irx. Unfortunately, the latter is _not_ open source, and
# we're not allowed to include it directly. As a workaround, get yourself a copy
# of usb_mass.irx, and set the environment variable USB_MASS to its file path,
# e.g. export USB_MASS=/usr/local/ps2dev/irx/usb_mass.irx
USB = 0

VARS=DEBUG=$(DEBUG) USB=$(USB)

all: check
	make -C iop
	bin2o iop/dev9/ps2dev9.irx ee/loader/ps2dev9_irx.o _ps2dev9_irx
	bin2o iop/smap/ps2smap.irx ee/loader/ps2smap_irx.o _ps2smap_irx
	bin2o iop/debugger/debugger.irx ee/loader/debugger_irx.o _debugger_irx
	bin2o iop/netlog/netlog.irx ee/loader/netlog_irx.o _netlog_irx
	bin2o iop/memdisk/memdisk.irx ee/loader/memdisk_irx.o _memdisk_irx
	bin2o iop/eesync/eesync.irx ee/loader/eesync_irx.o _eesync_irx
	bin2o $(PS2SDK)/iop/irx/ps2ip.irx ee/loader/ps2ip_irx.o _ps2ip_irx
	if [ $(USB) = "1" ]; then \
		bin2o $(PS2SDK)/iop/irx/usbd.irx ee/loader/usbd_irx.o _usbd_irx; \
		bin2o $(USB_MASS) ee/loader/usb_mass_irx.o _usb_mass_irx; \
	fi
	$(VARS) make -C ee

clean:
	$(VARS) make -C ee clean
	rm -f ee/loader/*_irx.o
	make -C iop clean
	rm -rf release/

rebuild: clean all

release: all
	rm -rf release
	mkdir -p release/bin
	if [ -x $(PS2DEV)/bin/ps2-packer ]; then \
		ps2-packer -v ee/loader/artemis.elf release/bin/artemis.elf; \
		chmod +x release/bin/artemis.elf; \
	else \
		cp ee/loader/artemis.elf release/bin/artemis.elf; \
	fi
	cp ee/loader/artemis.conf release/bin/
	cp ee/loader/cheats.txt release/bin/
	cp BUGS CHANGES COMMIT COPYING* CREDITS INSTALL README TODO release/
	cp -r doc/ release/

check:
	@if [ -z $(PS2SDK) ]; then \
		echo "PS2SDK env var not set." >&2; \
		exit 1; \
	fi
	@if [ $(USB) = "1" ] && [ -z $(USB_MASS) ]; then \
		echo "USB_MASS env var not set." >&2; \
		exit 1; \
	fi
