
  ifndef CC
   CC = gcc
  endif

  CFLAGS = -std=gnu99 -Wall -pedantic -I/usr/include -I/usr/local/include

  ifeq "x$(MSYSTEM)" "x"
   LIBS = -lpthread
  else
   LIBS = -lwsock32 -lpthreadGC2
  endif
  
  all: bin/ntpbclient

  clean:
	rm -f obj/*.o bin/*client*

  bin/ntpbclient: $(OFILES) src/ntpbclient.c
	@mkdir -p bin
	$(CC) $(CFLAGS) $(OFILES) src/ntpbclient.c -o bin/ntpbclient $(LIBS)
