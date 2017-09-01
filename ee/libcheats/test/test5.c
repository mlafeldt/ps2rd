/*
 * test GAMES/CHEATS/CODES macros
 */

#include <stdio.h>
#include <libcheats.h>

static void add_game(gamelist_t *list)
{
	game_t *game;
	cheat_t *cheat;
	code_t *code;

	game = make_game("Foo Game", NULL, 0);
	GAMES_INSERT_TAIL(list, game);

	cheat = make_cheat("Inf. Bar", NULL, 0);
	CHEATS_INSERT_TAIL(&game->cheats, cheat);

	code = make_code(0x123, 0x456, 0);
	CODES_INSERT_TAIL(&cheat->codes, code);
	code = make_code(0x789, 0, 0);
	CODES_INSERT_TAIL(&cheat->codes, code);
	remove_code(&cheat->codes, code, 1);
}

static void add_game2(gamelist_t *list)
{
	game_t *game;
	cheat_t *cheat;
	code_t *code;
	codelist_t codes = CODES_HEAD_INITIALIZER(codes);
	cheatlist_t cheats = CHEATS_HEAD_INITIALIZER(cheats);
	int i = 0;

	for (i = 0; i < 5; i++) {
		code = make_code(0x12345678, i, 0);
		CODES_INSERT_TAIL(&codes, code);
	}

	cheat = make_cheat("Inf. Bar 2", &codes, 0);
	CHEATS_INSERT_TAIL(&cheats, cheat);

	game = make_game("Foo Game 2", &cheats, 0);
	GAMES_INSERT_HEAD(list, game);
}

int test5(int argc, char *argv[])
{
	cheats_t cheats;

	cheats_init(&cheats);
	add_game(&cheats.games);
	add_game2(&cheats.games);
	cheats_write(&cheats, stdout);
	cheats_destroy(&cheats);

	return 0;
}
