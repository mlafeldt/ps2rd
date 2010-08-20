#
# This is the root makefile of PS2rd.
#

VERSION = $(shell git describe --tags --dirty 2>/dev/null || head -n1 CHANGES | cut -f1 -d " ")
PACKAGE = ps2rd-$(VERSION)

# Set DEBUG to 1 to enable the debug mode. In debug mode, a lot of helpful debug
# messages will be printed to "host:" when using ps2link.
DEBUG = 1

# Enable or disable netlog support (send log messages over UDP)
NETLOG = 0

# Set SMS_MODULES to 1 to build PS2rd with the network modules from SMS.
SMS_MODULES = 1

VARS = DEBUG=$(DEBUG) NETLOG=$(NETLOG) SMS_MODULES=$(SMS_MODULES)


SUBDIRS = ee iop pc

subdir_list  = $(SUBDIRS:%=all-%)
subdir_clean = $(SUBDIRS:%=clean-%)

.PHONY: $(SUBDIRS) $(subdir_list) $(subdir_clean) copy-irx all clean

.SILENT:

all: check $(subdir_list)

gen-version:
	echo "#define PS2RD_VERSION \"$(VERSION)\"" > ee/loader/version.h

all-ee: copy-irx gen-version

copy-irx: all-iop
	bin2o iop/debugger/debugger.irx ee/loader/debugger_irx.o _debugger_irx
	bin2o iop/dev9/ps2dev9.irx ee/loader/ps2dev9_irx.o _ps2dev9_irx
	bin2o iop/eesync/eesync.irx ee/loader/eesync_irx.o _eesync_irx
	bin2o iop/memdisk/memdisk.irx ee/loader/memdisk_irx.o _memdisk_irx
	bin2o $(PS2SDK)/iop/irx/usbd.irx ee/loader/usbd_irx.o _usbd_irx
	bin2o iop/usb_mass/usb_mass.irx ee/loader/usb_mass_irx.o _usb_mass_irx
	@if [ $(SMS_MODULES) = "1" ]; then \
		bin2o iop/SMSMAP/SMSMAP.irx ee/loader/ps2smap_sms_irx.o _ps2smap_sms_irx; \
		bin2o iop/SMSTCPIP/SMSTCPIP.irx ee/loader/ps2ip_sms_irx.o _ps2ip_sms_irx; \
	fi
	bin2o iop/smap/ps2smap.irx ee/loader/ps2smap_irx.o _ps2smap_irx
	bin2o $(PS2SDK)/iop/irx/ps2ip.irx ee/loader/ps2ip_irx.o _ps2ip_irx

clean: $(subdir_clean)
	rm -f ee/loader/*_irx.o
	rm -rf release/

$(subdir_list):
	$(VARS) $(MAKE) -C $(@:all-%=%)

$(subdir_clean):
	$(VARS) $(MAKE) -C $(@:clean-%=%) clean

run: all
	$(MAKE) -C ee/loader run

release: all
	echo "* Building $(PACKAGE) release packages ..."
	rm -rf release
	mkdir -p release/$(PACKAGE)/pc release/$(PACKAGE)/ps2
	@if [ -x $(PS2DEV)/bin/ps2-packer ]; then \
		ps2-packer -v ee/loader/ps2rd.elf release/$(PACKAGE)/ps2/ps2rd.elf; \
		chmod +x release/$(PACKAGE)/ps2/ps2rd.elf; \
	else \
		cp ee/loader/ps2rd.elf release/$(PACKAGE)/ps2/ps2rd.elf; \
	fi
	cp ee/loader/ps2rd.conf release/$(PACKAGE)/ps2/
	cp ee/loader/cheats.txt release/$(PACKAGE)/ps2/
	cp pc/ntpbclient/ntpbclient release/$(PACKAGE)/pc/
	cp BUGS CHANGES COPYING* CREDITS INSTALL README TODO release/$(PACKAGE)/
	cp -r Documentation/ release/$(PACKAGE)/
	cd release && \
		tar -cjf $(PACKAGE).tar.bz2 $(PACKAGE)/; \
		zip -qr $(PACKAGE).zip $(PACKAGE)/; \
		sha1sum $(PACKAGE).* > $(PACKAGE).sha1

check:
	@if [ -z $(PS2SDK) ]; then \
		echo "PS2SDK env var not set." >&2; \
		exit 1; \
	fi

help:
	@echo "The build targets are:"
	@echo " all     - compile project (default)"
	@echo " clean   - clean project"
	@echo " run     - launch executable with ps2client"
	@echo " check   - check for environment variables (invoked by all)"
	@echo " release - create release package"
