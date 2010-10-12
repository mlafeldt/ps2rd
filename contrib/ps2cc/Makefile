# Project: PS2CC

NAME  = ps2cc.exe

CC = gcc.exe
WINDRES = windres
RES  = ps2cc.res
OBJ  = ps2cc.o tab_memedit.o tab_results.o tab_search.o lib_api.o lib_fileio.o lib_ntpb.o lib_ps2cc.o lib_misc.o lib_search.o lib_dialog.o lib_listview.o $(RES)
LDFLAGS = -s -mwindows -lwsock32 -lcomctl32 -lkernel32
CFLAGS = -O2 -s

.PHONY: all

all: $(NAME)


clean:
	rm -f $(OBJ) $(NAME) $(NAME).exe

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) $(LDFLAGS)

ps2cc.o: ps2cc.c ps2cc.h ps2cc_gui.h _types.h
	$(CC) -c ps2cc.c -o ps2cc.o $(CFLAGS)

tab_results.o: tab_results.c ps2cc.h _types.h ps2cc_gui.h
	$(CC) -c tab_results.c -o tab_results.o $(CFLAGS)

tab_search.o: tab_search.c ps2cc.h _types.h ps2cc_gui.h
	$(CC) -c tab_search.c -o tab_search.o $(CFLAGS)

tab_memedit.o: tab_memedit.c ps2cc.h _types.h ps2cc_gui.h
	$(CC) -c tab_memedit.c -o tab_memedit.o $(CFLAGS)

lib_api.o: lib_api.c _types.h
	$(CC) -c lib_api.c -o lib_api.o $(CFLAGS)	

lib_dialog.o: lib_dialog.c _types.h ps2cc_gui.h
	$(CC) -c lib_dialog.c -o lib_dialog.o $(CFLAGS)	

lib_fileio.o: lib_fileio.c _types.h
	$(CC) -c lib_fileio.c -o lib_fileio.o $(CFLAGS)	

lib_listview.o: lib_listview.c _types.h
	$(CC) -c lib_listview.c -o lib_listview.o $(CFLAGS)	

lib_misc.o: lib_misc.c _types.h
	$(CC) -c lib_misc.c -o lib_misc.o $(CFLAGS)	

lib_ntpb.o: lib_ntpb.c _types.h
	$(CC) -c lib_ntpb.c -o lib_ntpb.o $(CFLAGS)	

lib_ps2cc.o: lib_ps2cc.c ps2cc.h ps2cc_gui.h _types.h
	$(CC) -c lib_ps2cc.c -o lib_ps2cc.o $(CFLAGS)	

lib_search.o: lib_search.c ps2cc.h _types.h
	$(CC) -c lib_search.c -o lib_search.o $(CFLAGS)

ps2cc.res: ps2cc.rc ps2cc_gui.h
	$(WINDRES) -i ps2cc.rc -J rc -o ps2cc.res -O coff

