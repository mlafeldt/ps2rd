/*
 * Boot loader (main project file)
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

#include <tamtypes.h>
#include <kernel.h>
#include <libcheats.h>
#include <libconfig.h>
#include <sbv_patches.h>
#include <sifrpc.h>
#include <string.h>
#include "cheatman.h"
#include "configman.h"
#include "dbgprintf.h"
#include "elfid.h"
#include "erlman.h"
#include "irxman.h"
#include "mycdvd.h"
#include "mypad.h"
#include "myutil.h"

#define APP_NAME	"PS2rd"
#define APP_VERSION	"0.5.3"
#define APP_BUILD_DATE	__DATE__" "__TIME__

#define WELCOME_STRING	"Welcome to "APP_NAME" "APP_VERSION"\n"

#define OPTIONS \
	"Options:\n" \
	"START | X  - Start game\n" \
	"LEFT RIGHT - Select boot ELF\n" \
	"UP DOWN    - Select game cheats\n" \
	"CIRCLE     - Auto-select cheats\n"

#define PAD_PORT	0
#define PAD_SLOT	0

static char _bootpath[FIO_PATH_MAX];
static enum dev_id _bootdev = DEV_UNKN;
static u8 _padbuf[256] __attribute__((aligned(64)));

/*
 * TODO use dirname() from libgen.h
 */

static inline size_t chr_idx(const char *s, char c)
{
	size_t i = 0;

	while (s[i] && (s[i] != c))
		i++;

	return (s[i] == c) ? i : -1;
}

static inline size_t last_chr_idx(const char *s, char c)
{
	size_t i = strlen(s);

	while (i && s[--i] != c)
		;

	return (s[i] == c) ? i : -1;
}

static int __dirname(char *filename)
{
	int i;

	if (filename == NULL)
		return -1;

	i = last_chr_idx(filename, '/');
	if (i < 0) {
		i = last_chr_idx(filename, '\\');
		if (i < 0) {
			i = chr_idx(filename, ':');
			if (i < 0)
				return -2;
		}
	}

	filename[i+1] = '\0';
	return 0;
}

/*
 * Build pathname based on boot device and filename.
 */
static char *__pathname(const char *name)
{
	static char filename[FIO_PATH_MAX];
	enum dev_id dev;

	filename[0] = '\0';
	dev = get_dev(name);

	/* Add boot path if name is relative */
	if (dev == DEV_UNKN)
		strcpy(filename, _bootpath);

	strcat(filename, name);

	if (dev == DEV_CD) {
		strupr(filename);
		strcat(filename, ";1");
	}

	return filename;
}

/*
 * Start ELF specified by @boot2, or parse SYSTEM.CNF for ELF if @boot2 is NULL.
 */
static int __start_elf(const char *boot2)
{
	char elfname[FIO_PATH_MAX];
	enum dev_id dev = DEV_CD;
	char *argv[16];
	int argc = 16;

	if (boot2 == NULL || (boot2 != NULL && (dev = get_dev(boot2)) == DEV_CD))
		_cdStandby(CDVD_NOBLOCK);

	if (boot2 == NULL) {
		if (cdGetElf(elfname) < 0) {
			A_PRINTF("Error: could not get ELF name from SYSTEM.CNF\n");
			_cdStop(CDVD_NOBLOCK);
			return -1;
		}
		boot2 = elfname;
	}

	build_argv(boot2, &argc, argv);

	if (!file_exists(argv[0])) {
		A_PRINTF("Error: ELF %s not found\n", argv[0]);
		if (dev == DEV_CD)
			_cdStop(CDVD_NOBLOCK);
		return -1;
	}

	A_PRINTF("Starting game...\n");
	D_PRINTF("%s: running ELF %s ...\n", __FUNCTION__, argv[0]);

	padPortClose(PAD_PORT, PAD_SLOT);
	padReset();

	LoadExecPS2(argv[0], argc-1, &argv[1]);

	if (dev == DEV_CD)
		_cdStop(CDVD_NOBLOCK);
	padInit(0);
	padPortOpen(PAD_PORT, PAD_SLOT, _padbuf);

	A_PRINTF("Error: could not load ELF %s\n", argv[0]);

	return -1;
}

int main(int argc, char *argv[])
{
	config_t config;
	cheats_t cheats;
	engine_t engine;
	const char *cheatfile = NULL;
	const char *boot2 = NULL;
	int select = 0;
	game_t *game = NULL;

	SifInitRpc(0);
	init_scr();
	scr_clear();

	A_PRINTF(WELCOME_STRING);
	D_PRINTF("Build date: "APP_BUILD_DATE"\n");

	strcpy(_bootpath, argv[0]);
	__dirname(_bootpath);
	_bootdev = get_dev(_bootpath);
	A_PRINTF("Booting from: %s\n", _bootpath);
	A_PRINTF("Initializing...\n");

	D_PRINTF("* Reading config...\n");
	config_build(&config);
	if (config_read_file(&config, __pathname(CONFIG_FILE)) != CONFIG_TRUE)
		D_PRINTF("config: %s\n", config_error_text(&config));
	config_print(&config);

	if (_bootdev != DEV_HOST && config_get_bool(&config, SET_IOP_RESET))
		reset_iop("rom0:UDNL rom0:EELOADCNF");

	if (config_get_bool(&config, SET_SBV_PATCHES)) {
		D_PRINTF("* Applying SBV patches...\n");
		sbv_patch_enable_lmb();
		sbv_patch_disable_prefix_check();
		sbv_patch_user_mem_clear(0x00100000);
	}

	if (load_modules(&config) < 0) {
		A_PRINTF("Error: failed to load IRX modules\n");
		goto end;
	}

	cdInit(CDVD_INIT_NOCHECK);
	_cdStop(CDVD_NOBLOCK);

	padInit(0);
	padPortOpen(PAD_PORT, PAD_SLOT, _padbuf);
	padWaitReady(PAD_PORT, PAD_SLOT);
	padSetMainMode(PAD_PORT, PAD_SLOT, PAD_MMODE_DIGITAL, PAD_MMODE_LOCK);

	if (install_erls(&config, &engine) < 0) {
		A_PRINTF("Error: failed to install ERLs\n");
		goto end;
	}
	install_modules(&config);

	cheats_init(&cheats);
	cheatfile = __pathname(config_get_string(&config, SET_CHEATS_FILE));
	if (load_cheats(cheatfile, &cheats) < 0)
		A_PRINTF("Error: failed to load cheats from %s\n", cheatfile);

	A_PRINTF(OPTIONS);
	A_PRINTF("Ready.\n");

	struct padButtonStatus btn;
	u32 old_pad = 0;
	while (1) {
		padWaitReady(PAD_PORT, PAD_SLOT);
		if (!padRead(PAD_PORT, PAD_SLOT, &btn))
			continue;
		u32 paddata = 0xFFFF ^ btn.btns;
		u32 new_pad = paddata & ~old_pad;
		old_pad = paddata;

		if ((new_pad & PAD_START) || (new_pad & PAD_CROSS)) {
			if (!config_get_bool(&config, SET_ENGINE_INSTALL)) {
				A_PRINTF("Skipping cheats - engine not installed\n");
			} else {
				reset_cheats(&engine);
				if (game != NULL) {
					A_PRINTF("Activate cheats for \"%s\"\n", game->title);
					activate_cheats(game, &engine);
				}
			}
			__start_elf(boot2);
		} else if (new_pad & PAD_SELECT) {
			/* do nothing */
		} else if (new_pad & PAD_CIRCLE) {
			game = find_cheats(boot2, &cheats);
			if (game != NULL)
				A_PRINTF("Auto-select \"%s\"\n", game->title);
			else
				A_PRINTF("No cheats found\n");
		} else if (new_pad & PAD_TRIANGLE) {
			/* for dev only */
			break;
		} else if ((new_pad & PAD_LEFT) || (new_pad & PAD_RIGHT)) {
			boot2 = (new_pad & PAD_RIGHT) ?
				config_get_string_elem(&config, SET_BOOT2, select++) : NULL;
			if (boot2 != NULL) {
				A_PRINTF("Select ELF \"%s\"\n", boot2);
			} else {
				A_PRINTF("Auto-select ELF from SYSTEM.CNF\n");
				select = 0;
			}
		} else if (new_pad & PAD_DOWN) {
			game = game ? GAMES_NEXT(game) : GAMES_FIRST(&cheats.games);
			if (game != NULL)
				A_PRINTF("Select \"%s\"\n", game->title);
			else
				A_PRINTF("Deselect all cheats\n");
		} else if (new_pad & PAD_UP) {
			game = NULL;
			A_PRINTF("Deselect all cheats\n");
		}
	}
end:
	A_PRINTF("Exit...\n");

	config_destroy(&config);
	cheats_destroy(&cheats);
	uninstall_erls();
	SleepThread();

	return 1;
}
