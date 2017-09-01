/*
 * test cheats_read_buf()
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

int test2(int argc, char *argv[])
{
	cheats_t cheats;

	cheats_init(&cheats);

	if (cheats_read_buf(&cheats, text) != CHEATS_TRUE) {
		printf("line: %i\nerror: %s\n",
			cheats.error_line, cheats.error_text);
		cheats_destroy(&cheats);
		return -1;
	}

	printf("cheats:\n\n");
	cheats_write(&cheats, stdout);
	cheats_destroy(&cheats);

	return 0;
}
