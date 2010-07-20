SUBDIRS =

subdir_list  = $(SUBDIRS:%=all-%)
subdir_clean = $(SUBDIRS:%=clean-%)

.PHONY: $(SUBDIRS) $(subdir_list) $(subdir_clean) all clean

.SILENT:

all: $(subdir_list)

clean: $(subdir_clean)

$(subdir_list):
	echo "* [PC] Building $(@:all-%=%) ..."
	$(MAKE) -C $(@:all-%=%)

$(subdir_clean):
	echo "* [PC] Cleaning $(@:clean-%=%) ..."
	$(MAKE) -C $(@:clean-%=%) clean
