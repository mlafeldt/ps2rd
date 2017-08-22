DOCKER_RUN  = docker run -it --rm -v "$(PWD):/src" mlafeldt/ps2dev:2011
DOCKER_MAKE = $(DOCKER_RUN) make -f Makefile.PS2 $(MAKEFLAGS)

all:
	$(DOCKER_MAKE)

clean:
	$(DOCKER_MAKE) $@

ee:
	$(DOCKER_MAKE) all-$@

iop:
	$(DOCKER_MAKE) all-$@

run: all
	ps2client reset
	sleep 5
	cd ee/loader && ps2client -t 3 execee host:ps2rd.elf

release:
	$(DOCKER_MAKE) $@

shell:
	$(DOCKER_RUN)

.PHONY: all clean ee iop run release shell
