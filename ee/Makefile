all:
	make -C engine
	make -C debugger
	bin2o -n engine/engine.erl loader/engine_erl.o _engine_erl
	bin2o -n debugger/debugger.erl loader/debugger_erl.o _debugger_erl
	bin2o -n $(PS2SDK)/ee/lib/libkernel.erl loader/libkernel_erl.o _libkernel_erl
	make -C loader

clean:
	make -C engine clean
	make -C debugger clean
	make -C loader clean

rebuild: clean all
