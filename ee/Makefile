ERL_MODULES=debugger elfldr engine videomod

all:
	@for m in $(ERL_MODULES); do \
		make -C $$m; \
		bin2o $${m}/$${m}.erl loader/$${m}_erl.o _$${m}_erl; \
	done
	bin2o $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl
	make -C loader

clean:
	@for m in $(ERL_MODULES) loader; do \
		make -C $$m clean; \
	done
	rm -f loader/*_erl.o

rebuild: clean all
