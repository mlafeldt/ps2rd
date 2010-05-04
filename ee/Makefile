.SILENT:

LIBCHEATS = "libcheats-0.1"
LIBCONFIG = "libconfig-1.4.4"

EE_LIBS = $(LIBCHEATS) $(LIBCONFIG)
EE_INCS = "-I$(CURDIR)/$(LIBCHEATS)/include -I$(CURDIR)/$(LIBCONFIG)/include"
EE_LDFLAGS = "-L$(CURDIR)/$(LIBCHEATS)/lib -L$(CURDIR)/$(LIBCONFIG)/lib"

ERL_MODULES = debugger elfldr engine videomod

all:
	@for m in $(EE_LIBS); do \
		echo "* [EE] Building lib $$m ..."; \
		$(MAKE) -C $$m -f Makefile.PS2; \
	done
	@for m in $(ERL_MODULES); do \
		echo "* [EE] Building erl $$m ..."; \
		$(MAKE) -C $$m; \
		bin2o $${m}/$${m}.erl loader/$${m}_erl.o _$${m}_erl; \
	done
	bin2o $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl
	echo "* [EE] Building loader ..."
	EE_INCS=$(EE_INCS) EE_LDFLAGS=$(EE_LDFLAGS) $(MAKE) -C loader

clean:
	@for m in $(EE_LIBS); do \
		echo "* [EE] Cleaning $$m ..."; \
		$(MAKE) -C $$m -f Makefile.PS2 clean; \
	done
	@for m in $(ERL_MODULES) loader; do \
		echo "* [EE] Cleaning $$m ..."; \
		$(MAKE) -C $$m clean; \
	done
	rm -f loader/*_erl.o

rebuild: clean all
