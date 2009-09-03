all:
	make -C dev9
	make -C smap
	make -C debugger
	make -C netlog
	make -C memdisk
	make -C eesync

clean:
	make -C dev9 clean
	make -C smap clean
	make -C debugger clean
	make -C netlog clean
	make -C memdisk clean
	make -C eesync clean

rebuild: clean all
