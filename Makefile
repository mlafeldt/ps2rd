all:
	make -C ee
	make -C iop

clean:
	make -C ee clean
	make -C iop clean

rebuild: clean all
