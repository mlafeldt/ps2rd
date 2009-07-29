/*
 * cheatlist.c - Cheat list handling
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 *
 * This file is part of libcheats.
 *
 * libcheats is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * libcheats is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libcheats.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include "cheatlist.h"

/**
 * make_code - Create a new code object.
 * @addr: code address
 * @val: code value
 * @tag: arbitrary information
 * @return: ptr to new code object, or NULL on mem alloc error
 */
code_t *make_code(uint32_t addr, uint32_t val, uint32_t tag)
{
	code_t *code = (code_t*)malloc(sizeof(code_t));

	if (code != NULL) {
		code->addr = addr;
		code->val = val;
		code->tag = tag;
	}

	return code;
}

/**
 * make_cheat - Create a new cheat object and populate it.
 * @desc: cheat description (can be NULL)
 * @codes: cheat codes (can be NULL)
 * @tag: arbitrary information
 * @return: ptr to new cheat object, or NULL on mem alloc error
 */
cheat_t *make_cheat(const char *desc, codelist_t *codes, uint32_t tag)
{
	cheat_t *cheat = (cheat_t*)malloc(sizeof(cheat_t));

	if (cheat != NULL) {
		if (desc != NULL)
			strncpy(cheat->desc, desc, CHEAT_DESC_MAX);
		else
			strcpy(cheat->desc, "-");

		CODES_INIT(&cheat->codes);
		if (codes != NULL)
			CODES_CONCAT(&cheat->codes, codes);

		cheat->tag = tag;
	}

	return cheat;
}

/**
 * make_game - Create a new game object and populate it.
 * @title: game title (can be NULL)
 * @cheats: game cheats (can be NULL)
 * @tag: arbitrary information
 * @return: ptr to new game object, or NULL on mem alloc error
 */
game_t *make_game(const char *title, cheatlist_t *cheats, uint32_t tag)
{
	game_t *game = (game_t*)malloc(sizeof(game_t));

	if (game != NULL) {
		if (title != NULL)
			strncpy(game->title, title, GAME_TITLE_MAX);
		else
			strcpy(game->title, "-");

		CHEATS_INIT(&game->cheats);
		if (cheats != NULL)
			CHEATS_CONCAT(&game->cheats, cheats);

		game->tag = tag;
	}

	return game;
}

static inline void __remove_codes(codelist_t *list, int _free)
{
	code_t *code;

	while ((code = CODES_FIRST(list)) != NULL) {
		CODES_REMOVE(list, code);
		if (_free)
			free(code);
	}
}

static inline void __remove_cheats(cheatlist_t *list, int _free)
{
	cheat_t *cheat;

	while ((cheat = CHEATS_FIRST(list)) != NULL) {
		__remove_codes(&cheat->codes, _free);
		CHEATS_REMOVE(list, cheat);
		if (_free)
			free(cheat);
	}
}

static inline void __remove_games(gamelist_t *list, int _free)
{
	game_t *game;

	while ((game = GAMES_FIRST(list)) != NULL) {
		__remove_cheats(&game->cheats, _free);
		GAMES_REMOVE(list, game);
		if (_free)
			free(game);
	}
}

/*
 * remove_code - Remove a code from a code list and optionally free its memory.
 */
void remove_code(codelist_t *list, code_t *code, int _free)
{
	CODES_REMOVE(list, code);
	if (_free)
		free(code);
}

/*
 * remove_cheat - Remove a cheat from a cheat list and optionally free its memory.
 */
void remove_cheat(cheatlist_t *list, cheat_t *cheat, int _free)
{
	__remove_codes(&cheat->codes, _free);
	CHEATS_REMOVE(list, cheat);
	if (_free)
		free(cheat);
}

/*
 * remove_game - Remove a game from a game list and optionally free its memory.
 */
void remove_game(gamelist_t *list, game_t *game, int _free)
{
	__remove_cheats(&game->cheats, _free);
	GAMES_REMOVE(list, game);
	if (_free)
		free(game);
}

/*
 * free_codes - Free an entire code list.
 */
void free_codes(codelist_t *list)
{
	__remove_codes(list, 1);
}

/*
 * free_cheats - Free an entire cheat list.
 */
void free_cheats(cheatlist_t *list)
{
	__remove_cheats(list, 1);
}

/*
 * free_games - Free an entire game list.
 */
void free_games(gamelist_t *list)
{
	__remove_games(list, 1);
}

/**
 * sort_games - Sort a game list by title.
 * @list: list to be sorted
 */
void sort_games(gamelist_t *list)
{
	/* TODO */
}

/**
 * find_game_by_title - Search a game list for a game with a certain title.
 * @title: title to search for
 * @list: list that is searched
 * @return: ptr to found game object, or NULL if it could not be found
 */
game_t *find_game_by_title(const char *title, const gamelist_t *list)
{
	game_t *game;

	GAMES_FOREACH(game, list) {
		if (!strcmp(game->title, title))
			return game;
	}

	return NULL;
}
