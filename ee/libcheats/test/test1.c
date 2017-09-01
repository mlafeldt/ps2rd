/*
 * test cheats_read_file()
 */

#include <stdio.h>
#include <libcheats.h>

int test1(int argc, char *argv[])
{
	cheats_t cheats;

	if (argc < 2)
		return -1;

	cheats_init(&cheats);

	if (cheats_read_file(&cheats, argv[1]) != CHEATS_TRUE) {
		printf("line: %i\nerror: %s\n",
			cheats.error_line, cheats.error_text);
		cheats_destroy(&cheats);
		return -1;
	}

	cheats_destroy(&cheats);
	return 0;
}
