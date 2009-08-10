all:
	make -C iop
	bin2o iop/dev9/ps2dev9.irx ee/loader/ps2dev9_irx.o _ps2dev9_irx
	bin2o iop/smap/ps2smap.irx ee/loader/ps2smap_irx.o _ps2smap_irx
	bin2o iop/debugger/debugger.irx ee/loader/debugger_irx.o _debugger_irx
	bin2o $(PS2SDK)/iop/irx/ps2ip.irx ee/loader/ps2ip_irx.o _ps2ip_irx
	make -C ee

clean:
	make -C ee clean
	rm -f ee/loader/*_irx.o
	make -C iop clean
	rm -rf release/

rebuild: clean all

release: all
	rm -rf release
	mkdir release
	cp ee/loader/artemis.elf release/
	cp ee/loader/artemis.conf release/
	cp ee/loader/cheats.txt release/
	cp BUGS CHANGES COPYING* CREDITS INSTALL README TODO release/
