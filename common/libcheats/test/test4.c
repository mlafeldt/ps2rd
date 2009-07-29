/*
 * test freeing of memory
 */

#include <stdio.h>
#include <libcheats.h>

static const char *text =
"\"TimeSplitters 2 Demo PAL\"\n"
"Mastercode\n"
"F02000A4 0000000E\n"
"Inf. Schnubbel\n"
"0021FF44 00000002\n"
"\"Tom Clancy's Splinter Cell PAL\"\n"
"Mastercode\n"
"90129A84 0C048534\n"
"Unlock all Levels\n"
"20383A30 00000000\n";

int test4(int argc, char *argv[])
{
	cheats_t cheats;
	int i;

	for (i = 1; i <= 10; i++) {
		cheats_init(&cheats);

		if (cheats_read_buf(&cheats, text) != CHEATS_TRUE) {
			printf("line: %i\nerror: %s\n",
				cheats.error_line, cheats.error_text);
			cheats_destroy(&cheats);
			return -1;
		}

		printf("---------- free test #%i ----------\n", i);
		cheats_destroy(&cheats);
	}

	return 0;
}
