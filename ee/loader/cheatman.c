/*
 * Manage cheat codes
 *
 * Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
 *
 * This file is part of PS2rd, the PS2 remote debugger.
 *
 * PS2rd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PS2rd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PS2rd.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include <stdio.h>
#include <libcheats.h>
#include "elfid.h"
#include "engine.h"
#include "mycdvd.h"
#include "myutil.h"

#define GAME_ID_START	"/ID"
#define GAME_ID_WC	'?'

/*
 * Load cheats from text file.
 */
int load_cheats(const char *cheatfile, cheats_t *cheats)
{
	char *buf = NULL;
	int ret;

	cheats_destroy(cheats);

	buf = read_text_file(cheatfile, 0);
	if (buf == NULL) {
		fprintf(stderr, "%s: could not read cheats file '%s'\n",
			__FUNCTION__, cheatfile);
		return -1;
	}

	cheats_init(cheats);
	ret = cheats_read_buf(cheats, buf);
	free(buf);
	if (ret != CHEATS_TRUE) {
		fprintf(stderr, "%s: line %i: %s\n", cheatfile,
			cheats->error_line, cheats->error_text);
		cheats_destroy(cheats);
		return -1;
	}

	return 0;
}

/*
 * Find cheats for a game by its elf id.
 */
static game_t *__find_cheats(const elfid_t *id, const gamelist_t *list)
{
	game_t *game;
	elfid_t id2;
	char *p = NULL;

	GAMES_FOREACH(game, list) {
		p = strstr(game->title, GAME_ID_START);
		if (p != NULL && !elfid_parse(p + strlen(GAME_ID_START), &id2)) {
			if (!elfid_compare(id, &id2, GAME_ID_WC))
				return game;
		}
	}

	return NULL;
}

/*
 * Find cheats for given ELF.
 */
game_t *find_cheats(const char *boot2, const cheats_t *cheats)
{
	char elfname[FIO_PATH_MAX];
	enum dev_id dev = DEV_CD;
	char *argv[1];
	int argc = 1;
	elfid_t id;

	if (boot2 == NULL || (boot2 != NULL && (dev = get_dev(boot2)) == DEV_CD)) {
		_cdStandby(CDVD_NOBLOCK);
		delay(100);
	}

	if (boot2 == NULL) {
		if (cdGetElf(elfname) < 0) {
			fprintf(stderr, "%s: could not get ELF name from SYSTEM.CNF\n",
				__FUNCTION__);
			_cdStop(CDVD_NOBLOCK);
			return NULL;
		}
		boot2 = elfname;
	}

	build_argv(boot2, &argc, argv);

	if (elfid_generate(argv[0], &id) < 0) {
		fprintf(stderr, "%s: could not generate ID from ELF %s\n",
			__FUNCTION__, argv[0]);
		if (dev == DEV_CD)
			_cdStop(CDVD_NOBLOCK);
		return NULL;
	}

	if (dev == DEV_CD)
		_cdStop(CDVD_NOBLOCK);

	return __find_cheats(&id, &cheats->games);
}

/*
 * Activate selected cheats.
 */
void activate_cheats(const game_t *game, engine_t *engine)
{
	cheat_t *cheat = NULL;
	code_t *code = NULL;

	CHEATS_FOREACH(cheat, &game->cheats) {
		CODES_FOREACH(code, &cheat->codes) {
			D_PRINTF("%08X %08X\n", code->addr, code->val);
			/* TODO improve check for hook */
			if ((code->addr & 0xfe000000) == 0x90000000)
				engine_add_hook(engine, code->addr, code->val);
			else
				engine_add_code(engine, code->addr, code->val);
		}
	}
}

/*
 * Reset activated cheats.
 */
void reset_cheats(engine_t *engine)
{
	engine_clear_hooks(engine);
	engine_clear_codes(engine);
}
