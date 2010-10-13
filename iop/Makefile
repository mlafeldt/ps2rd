SUBDIRS = debugger dev9 eesync memdisk usb_mass smap

ifeq ($(SMS_MODULES),1)
SUBDIRS += SMSMAP SMSTCPIP
endif

subdir_list  = $(SUBDIRS:%=all-%)
subdir_clean = $(SUBDIRS:%=clean-%)

.PHONY: $(SUBDIRS) $(subdir_list) $(subdir_clean) all clean

ifndef VERBOSE
.SILENT:
endif

all: $(subdir_list)

clean: $(subdir_clean)

$(subdir_list):
	echo "* [IOP] Building $(@:all-%=%) ..."
	$(MAKE) -C $(@:all-%=%)

$(subdir_clean):
	echo "* [IOP] Cleaning $(@:clean-%=%) ..."
	$(MAKE) -C $(@:clean-%=%) clean
