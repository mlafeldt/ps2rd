LIBCHEATS_DIR = libcheats
LIBCHEATS = $(CURDIR)/$(LIBCHEATS_DIR)
export LIBCHEATS

LIBCONFIG_DIR = libconfig
LIBCONFIG = $(CURDIR)/$(LIBCONFIG_DIR)
export LIBCONFIG

EE_LIBS = $(LIBCHEATS_DIR) $(LIBCONFIG_DIR)
ERL_MODULES = debugger elfldr engine
SUBDIRS = $(EE_LIBS) $(ERL_MODULES) loader

subdir_list  = $(SUBDIRS:%=all-%)
subdir_clean = $(SUBDIRS:%=clean-%)

.PHONY: $(SUBDIRS) $(subdir_list) $(subdir_clean) copy-erl all clean

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
	$(MAKE) -C $(@:all-%=%)

$(subdir_clean):
	echo "* [EE] Cleaning $(@:clean-%=%) ..."
	$(MAKE) -C $(@:clean-%=%) clean
