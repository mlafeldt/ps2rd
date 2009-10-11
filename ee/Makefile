all:
	make -C engine
	bin2o engine/engine.erl loader/engine_erl.o _engine_erl
	make -C debugger
	bin2o debugger/debugger.erl loader/debugger_erl.o _debugger_erl
	make -C elfldr
	bin2o elfldr/elfldr.erl loader/elfldr_erl.o _elfldr_erl
	make -C videomod
	bin2o videomod/videomod.erl loader/videomod_erl.o _videomod_erl
	bin2o $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	bin2o $(PS2SDK)/ee/lib/libpatches.erl loader/libpatches_erl.o _libpatches_erl
	make -C loader

clean:
	make -C engine clean
	make -C debugger clean
	make -C elfldr clean
	make -C videomod clean
	make -C loader clean
	rm -f loader/*_erl.o

rebuild: clean all
