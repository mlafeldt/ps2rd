/*
 * cheatlist.h - Cheat list handling
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

#ifndef _CHEATLIST_H_
#define _CHEATLIST_H_

#include "libcheats_compat.h"

/*
 * Here's how everything is organized.  The current implementation uses single-
 * linked tail queues to store games, cheats, and codes.
 *
 * gamelist
 * |- game
 * |  |- cheatlist
 * |  |  |- cheat
 * |  |  |  |- codelist
 * |  |  |  |  |- code
 * |  |  |  |  |- code
 * |  |  |  |  |- ...
 * |  |  |- cheat
 * |  |  |  |- codelist
 * |  |  |  |  |- code
 * |  |  |  |  |- code
 * |  |  |  |  |- ...
 * |  |  |- ...
 * |- game
 * |- ...
 */

/*
 * Some helper macros.
 */
#define _FIELD	node
#define _STAILQ_INSERT_HEAD(h, e)	STAILQ_INSERT_HEAD(h, e, _FIELD)
#define _STAILQ_INSERT_TAIL(h, e)	STAILQ_INSERT_TAIL(h, e, _FIELD)
#define _STAILQ_INSERT_AFTER(h, l, e)	STAILQ_INSERT_AFTER(h, l, e, _FIELD)
#define _STAILQ_REMOVE(h, e, t)		STAILQ_REMOVE(h, e, t, _FIELD)
#define _STAILQ_REMOVE_HEAD(h)		STAILQ_REMOVE_HEAD(h, _FIELD)
#define _STAILQ_FOREACH(v, h)		STAILQ_FOREACH(v, h, _FIELD)
#define _STAILQ_NEXT(e)			STAILQ_NEXT(e, _FIELD)


/*
 * Code defines.
 *
 * NOTE: Use remove_code() or free_codes() to free allocated memory.
 * TODO: Use list instead of tail queue.
 */
#define CODES_HEAD_INITIALIZER(head)		STAILQ_HEAD_INITIALIZER(head)
#define CODES_INIT(head)			STAILQ_INIT(head)
#define CODES_INSERT_HEAD(head, elm)		_STAILQ_INSERT_HEAD(head, elm)
#define CODES_INSERT_TAIL(head, elm)		_STAILQ_INSERT_TAIL(head, elm)
#define CODES_INSERT_AFTER(head, listelm, elm)	_STAILQ_INSERT_AFTER(head, listelm, elm)
#define CODES_REMOVE(head, elm)			_STAILQ_REMOVE(head, elm, _code)
#define CODES_REMOVE_HEAD(head)			_STAILQ_REMOVE_HEAD(head)
#define CODES_FOREACH(var, head)		_STAILQ_FOREACH(var, head)
#define CODES_CONCAT(head1, head2)		STAILQ_CONCAT(head1, head2)
#define CODES_EMPTY(head)			STAILQ_EMPTY(head)
#define CODES_FIRST(head)			STAILQ_FIRST(head)
#define CODES_NEXT(elm)				_STAILQ_NEXT(elm)

/**
 * code_t - a code object
 * @addr: code address
 * @val: code value
 * @tag: arbitrary information
 */
typedef struct _code {
	uint32_t	addr;
	uint32_t	val;
	uint32_t	tag;

	STAILQ_ENTRY(_code) _FIELD;
} code_t;

typedef STAILQ_HEAD(_codelist, _code) codelist_t;


/*
 * Cheat defines.
 *
 * NOTE: Use remove_cheat() or free_cheats() to free allocated memory.
 */
#define CHEATS_HEAD_INITIALIZER(head)		STAILQ_HEAD_INITIALIZER(head)
#define CHEATS_INIT(head)			STAILQ_INIT(head)
#define CHEATS_INSERT_HEAD(head, elm)		_STAILQ_INSERT_HEAD(head, elm)
#define CHEATS_INSERT_TAIL(head, elm)		_STAILQ_INSERT_TAIL(head, elm)
#define CHEATS_INSERT_AFTER(head, listelm, elm)	_STAILQ_INSERT_AFTER(head, listelm, elm)
#define CHEATS_REMOVE(head, elm)		_STAILQ_REMOVE(head, elm, _cheat)
#define CHEATS_REMOVE_HEAD(head)		_STAILQ_REMOVE_HEAD(head)
#define CHEATS_FOREACH(var, head)		_STAILQ_FOREACH(var, head)
#define CHEATS_CONCAT(head1, head2)		STAILQ_CONCAT(head1, head2)
#define CHEATS_EMPTY(head)			STAILQ_EMPTY(head)
#define CHEATS_FIRST(head)			STAILQ_FIRST(head)
#define CHEATS_NEXT(elm)			_STAILQ_NEXT(elm)

/* Max cheat description length */
#define CHEAT_DESC_MAX	80

/**
 * cheat_t - a cheat object
 * @desc: cheat description
 * @codes: cheat codes
 * @tag: arbitrary information
 */
typedef struct _cheat {
	char		desc[CHEAT_DESC_MAX + 1];
	codelist_t	codes;
	uint32_t	tag;

	STAILQ_ENTRY(_cheat) _FIELD;
} cheat_t;

typedef STAILQ_HEAD(_cheatlist, _cheat) cheatlist_t;


/*
 * Game defines.
 *
 * NOTE: Use remove_game() or free_games() to free allocated memory.
 */
#define GAMES_HEAD_INITIALIZER(head)		STAILQ_HEAD_INITIALIZER(head)
#define GAMES_INIT(head)			STAILQ_INIT(head)
#define GAMES_INSERT_HEAD(head, elm)		_STAILQ_INSERT_HEAD(head, elm)
#define GAMES_INSERT_TAIL(head, elm)		_STAILQ_INSERT_TAIL(head, elm)
#define GAMES_INSERT_AFTER(head, listelm, elm)	_STAILQ_INSERT_AFTER(head, listelm, elm)
#define GAMES_REMOVE(head, elm)			_STAILQ_REMOVE(head, elm, _game)
#define GAMES_REMOVE_HEAD(head)			_STAILQ_REMOVE_HEAD(head)
#define GAMES_FOREACH(var, head)		_STAILQ_FOREACH(var, head)
#define GAMES_CONCAT(head1, head2)		STAILQ_CONCAT(head1, head2)
#define GAMES_EMPTY(head)			STAILQ_EMPTY(head)
#define GAMES_FIRST(head)			STAILQ_FIRST(head)
#define GAMES_NEXT(elm)				_STAILQ_NEXT(elm)

/* Max game title length */
#define GAME_TITLE_MAX	80

/**
 * game_t - a game object
 * @title: game title
 * @cheats: game cheats
 * @tag: arbitrary information
 */
typedef struct _game {
	char		title[GAME_TITLE_MAX + 1];
	cheatlist_t	cheats;
	uint32_t	tag;

	STAILQ_ENTRY(_game) _FIELD;
} game_t;

typedef STAILQ_HEAD(_gamelist, _game) gamelist_t;


/*
 * Some utility functions.
 */

/**
 * make_code - Create a new code object.
 * @addr: code address
 * @val: code value
 * @tag: arbitrary information
 * @return: ptr to new code object, or NULL on mem alloc error
 */
extern code_t *make_code(uint32_t addr, uint32_t val, uint32_t tag);

/**
 * make_cheat - Create a new cheat object and populate it.
 * @desc: cheat description (can be NULL)
 * @codes: cheat codes (can be NULL)
 * @tag: arbitrary information
 * @return: ptr to new cheat object, or NULL on mem alloc error
 */
extern cheat_t *make_cheat(const char *desc, codelist_t *codes, uint32_t tag);

/**
 * make_game - Create a new game object and populate it.
 * @title: game title (can be NULL)
 * @cheats: game cheats (can be NULL)
 * @tag: arbitrary information
 * @return: ptr to new game object, or NULL on mem alloc error
 */
extern game_t *make_game(const char *title, cheatlist_t *cheats, uint32_t tag);

/*
 * remove_code - Remove a code from a code list and optionally free its memory.
 */
extern void remove_code(codelist_t *list, code_t *code, int _free);

/*
 * remove_cheat - Remove a cheat from a cheat list and optionally free its memory.
 */
extern void remove_cheat(cheatlist_t *list, cheat_t *cheat, int _free);

/*
 * remove_game - Remove a game from a game list and optionally free its memory.
 */
extern void remove_game(gamelist_t *list, game_t *game, int _free);

/*
 * free_codes - Free an entire code list.
 */
extern void free_codes(codelist_t *list);

/*
 * free_cheats - Free an entire cheat list.
 */
extern void free_cheats(cheatlist_t *list);

/*
 * free_games - Free an entire game list.
 */
extern void free_games(gamelist_t *list);

/**
 * find_game_by_title - Search a game list for a game with a certain title.
 * @title: title to search for
 * @list: list that is searched
 * @return: ptr to found game object, or NULL if it could not be found
 */
extern game_t *find_game_by_title(const char *title, const gamelist_t *list);

#endif /* _CHEATLIST_H_ */
