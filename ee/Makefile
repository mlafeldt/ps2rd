.SILENT:

ERL_MODULES = debugger elfldr engine videomod

all:
	@for m in $(ERL_MODULES); do \
		echo "* [EE] Building $$m ..."; \
		$(MAKE) -C $$m; \
		bin2o $${m}/$${m}.erl loader/$${m}_erl.o _$${m}_erl; \
	done
	bin2o $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl
	echo "* [EE] Building loader ..."
	$(MAKE) -C loader

clean:
	@for m in $(ERL_MODULES) loader; do \
		echo "* [EE] Cleaning $$m ..."; \
		$(MAKE) -C $$m clean; \
	done
	rm -f loader/*_erl.o

rebuild: clean all
