LIBCHEATS = libcheats-0.1
LIBCONFIG = libconfig-1.4.5

EE_LIBS = $(LIBCHEATS) $(LIBCONFIG)
VARS = LIBCHEATS=$(CURDIR)/$(LIBCHEATS) LIBCONFIG=$(CURDIR)/$(LIBCONFIG)

ERL_MODULES = debugger elfldr engine videomod

lib_list  = $(EE_LIBS:%=all-%)
lib_clean = $(EE_LIBS:%=clean-%)

erl_list  = $(ERL_MODULES:%=all-%)
erl_clean = $(ERL_MODULES:%=clean-%)

.PHONY: $(EE_LIBS) $(ERL_MODULES) $(lib_list) $(lib_clean) $(erl_list) $(erl_clean)

.SILENT:

all: $(lib_list) $(erl_list)
	echo "* [EE] Building elf loader ..."
	@for i in $(ERL_MODULES); do \
		bin2o $${i}/$${i}.erl loader/$${i}_erl.o _$${i}_erl; \
	done
	bin2o $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl
	$(VARS) $(MAKE) -C loader

clean: $(lib_clean) $(erl_clean)
	rm -f loader/*_erl.o

$(lib_list):
	echo "* [EE] Building lib $(@:all-%=%) ..."
	$(MAKE) -C $(@:all-%=%) -f Makefile.PS2

$(lib_clean):
	echo "* [EE] Cleaning lib $(@:clean-%=%) ..."
	$(MAKE) -C $(@:clean-%=%) -f Makefile.PS2 clean

$(erl_list):
	echo "* [EE] Building erl $(@:all-%=%) ..."
	$(MAKE) -C $(@:all-%=%)

$(erl_clean):
	echo "* [EE] Cleaning erl $(@:clean-%=%) ..."
	$(MAKE) -C $(@:clean-%=%) clean
