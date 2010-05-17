.SILENT:

LIBCHEATS = "libcheats-0.1"
LIBCONFIG = "libconfig-1.4.4"

EE_LIBS = $(LIBCHEATS) $(LIBCONFIG)
VARS = LIBCHEATS=$(CURDIR)/$(LIBCHEATS) LIBCONFIG=$(CURDIR)/$(LIBCONFIG)

ERL_MODULES = debugger elfldr engine videomod

all:
	@for i in $(EE_LIBS); do \
		echo "* [EE] Building lib $$i ..."; \
		$(MAKE) -C $$i -f Makefile.PS2; \
	done
	@for i in $(ERL_MODULES); do \
		echo "* [EE] Building erl $$i ..."; \
		$(MAKE) -C $$i; \
		bin2o $${i}/$${i}.erl loader/$${i}_erl.o _$${i}_erl; \
	done
	bin2o $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl
	echo "* [EE] Building elf loader ..."
	$(VARS) $(MAKE) -C loader

clean:
	@for i in $(EE_LIBS); do \
		echo "* [EE] Cleaning $$i ..."; \
		$(MAKE) -C $$i -f Makefile.PS2 clean; \
	done
	@for i in $(ERL_MODULES) loader; do \
		echo "* [EE] Cleaning $$i ..."; \
		$(MAKE) -C $$i clean; \
	done
	rm -f loader/*_erl.o
