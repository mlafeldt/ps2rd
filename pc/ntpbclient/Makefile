
ifndef CC
CC = gcc
endif

CFLAGS = -std=gnu99 -pedantic -Wall -O2 -fno-strict-aliasing -s -I/usr/include -I/usr/local/include

ifneq "x$(MSYSTEM)" "x"
LIBS = -lwsock32
endif
  
all: bin/ntpbclient

clean:
	rm -f bin/*client* src/*.o

bin/ntpbclient: src/ntpbclient.o
	@mkdir -p bin
	$(CC) $(CFLAGS) src/ntpbclient.c -o bin/ntpbclient $(LIBS)
