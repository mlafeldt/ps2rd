all: build_loader

build_engine:
	make -C engine

build_debugger:
	make -C debugger

build_loader: build_engine build_debugger
	bin2o -n engine/engine.erl loader/engine_erl.o _binary_engine_erl
	bin2o -n debugger/debugger.erl loader/debugger_erl.o _binary_debugger_erl
	make -C loader

clean:
	make -C engine clean
	make -C debugger clean
	make -C loader clean

rebuild: clean all
