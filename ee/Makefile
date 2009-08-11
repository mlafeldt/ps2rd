all:
	make -C engine
	bin2o -n engine/engine.erl loader/engine_erl.o _engine_erl
	make -C debugger
	bin2o -n debugger/debugger.erl loader/debugger_erl.o _debugger_erl
	make -C elfldr
	bin2o -n elfldr/elfldr.elf loader/elfldr_elf.o _elfldr_elf
	make -C elfldr_erl
	bin2o -n elfldr_erl/elfldr.erl loader/elfldr_erl.o _elfldr_erl
	bin2o -n $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o -n $(PS2SDK)/ee/lib/libc.erl loader/libc_erl.o _libc_erl
	bin2o -n $(PS2SDK)/ee/lib/libdebug.erl loader/libdebug_erl.o _libdebug_erl
	bin2o -n $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl
	make -C loader

clean:
	make -C engine clean
	make -C debugger clean
	make -C elfldr clean
	make -C elfldr_erl clean
	make -C loader clean
	rm -f loader/*_erl.o loader/*_elf.o

rebuild: clean all
