LIBCHEATS = libcheats-0.1
LIBCONFIG = libconfig-1.4.5

VARS = LIBCHEATS=$(CURDIR)/$(LIBCHEATS) LIBCONFIG=$(CURDIR)/$(LIBCONFIG)

EE_LIBS = $(LIBCHEATS) $(LIBCONFIG)
ERL_MODULES = debugger elfldr engine videomod
SUBDIRS = $(EE_LIBS) $(ERL_MODULES) loader

subdir_list  = $(SUBDIRS:%=all-%)
subdir_clean = $(SUBDIRS:%=clean-%)

.PHONY: $(SUBDIRS) $(subdir_list) $(subdir_clean) copy-erl all clean

.SILENT:

all: $(subdir_list)

all-loader: $(addprefix all-,$(EE_LIBS)) copy-erl

copy-erl: $(addprefix all-,$(ERL_MODULES))
	@for i in $(ERL_MODULES); do \
		bin2o $${i}/$${i}.erl loader/$${i}_erl.o _$${i}_erl; \
	done
	bin2o $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl

clean: $(subdir_clean)
	rm -f loader/*_erl.o

$(subdir_list):
	echo "* [EE] Building $(@:all-%=%) ..."
	$(VARS) $(MAKE) -C $(@:all-%=%)

$(subdir_clean):
	echo "* [EE] Cleaning $(@:clean-%=%) ..."
	$(VARS) $(MAKE) -C $(@:clean-%=%) clean
