
VARS=DEBUG=$(DEBUG)

all:
	$(VARS) make -C engine
	bin2o engine/engine.erl loader/engine_erl.o _engine_erl
	$(VARS) make -C debugger
	bin2o debugger/debugger.erl loader/debugger_erl.o _debugger_erl
	$(VARS) make -C elfldr
	bin2o elfldr/elfldr.erl loader/elfldr_erl.o _elfldr_erl
	bin2o $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o $(PS2SDK)/ee/lib/libc.erl loader/libc_erl.o _libc_erl
	bin2o $(PS2SDK)/ee/lib/libdebug.erl loader/libdebug_erl.o _libdebug_erl
	bin2o $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl
	$(VARS) make -C loader

clean:
	$(VARS) make -C engine clean
	$(VARS) make -C debugger clean
	$(VARS) make -C elfldr clean
	$(VARS) make -C loader clean
	rm -f loader/*_erl.o

rebuild: clean all
