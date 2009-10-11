/*
 * loader.c - boot loader (main project file)
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 *
 * This file is part of Artemis, the PS2 game debugger.
 *
 * Artemis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Artemis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Artemis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sbv_patches.h>
#include <sifrpc.h>
#include <string.h>
#include <libcheats.h>
#include <libconfig.h>
#include "configman.h"
#include "dbgprintf.h"
#include "erlman.h"
#include "gameid.h"
#include "irxman.h"
#include "mycdvd.h"
#include "mypad.h"
#include "mystring.h"
#include "myutil.h"

#define APP_NAME	"Artemis"
#define APP_VERSION	"0.4 WIP"
#define APP_BUILD_DATE	__DATE__" "__TIME__

#define WELCOME_STRING	"Welcome to "APP_NAME" "APP_VERSION"\n"

#define OPTIONS \
	"Options:\n" \
	"START | X - Start Game\n" \
	"SELECT    - Select Boot ELF\n" \
	"CIRCLE    - Activate Cheats\n"

#define PAD_PORT	0
#define PAD_SLOT	0

#ifndef CONFIG_FILE
#define CONFIG_FILE	"artemis.conf"
#endif

/* Boot information */
static char g_bootpath[FIO_PATH_MAX];
static enum dev g_bootdev = DEV_UNKN;

static u8 g_padbuf[256] __attribute__((aligned(64)));

/*
 * Build pathname based on boot device and filename.
 */
static char *__pathname(const char *name)
{
	static char filename[FIO_PATH_MAX];
	enum dev dev;

	filename[0] = '\0';
	dev = get_dev(name);

	/* Add boot path if name is relative */
	if (dev == DEV_UNKN)
		strcpy(filename, g_bootpath);

	strcat(filename, name);

	if (dev == DEV_CD) {
		to_upper_str(filename);
		strcat(filename, ";1");
	}

	return filename;
}

/*
 * Load cheats from text file.
 */
static int load_cheats(const config_t *config, cheats_t *cheats)
{
	const char *cheatfile = _config_get_string(config, SET_CHEATS_FILE);
	char *buf = NULL;
	int ret;

	cheats_destroy(cheats);

	buf = read_text_file(__pathname(cheatfile), 0);
	if (buf == NULL) {
		A_PRINTF("Error: could not read cheats file '%s'\n", cheatfile);
		return -1;
	}

	cheats_init(cheats);
	ret = cheats_read_buf(cheats, buf);
	free(buf);
	if (ret != CHEATS_TRUE) {
		A_PRINTF("%s: line %i: %s\n", cheatfile, cheats->error_line,
			cheats->error_text);
		cheats_destroy(cheats);
		return -1;
	}

	return 0;
}

/*
 * Build argument vector from string @s.
 */
static void get_argv(const char *s, int *argc, char *argv[])
{
	static char argbuf[256];
	int max;
	char *tok;

	max = *argc;
	*argc = 0;

	memset(argv, 0, max * sizeof(argv[0]));
	memset(argbuf, 0, sizeof(argbuf));
	strncpy(argbuf, s, sizeof(argbuf)-1);

	tok = strtok(argbuf, "\t ");
	while (tok != NULL && *argc < max) {
		D_PRINTF("%s: argv[%i] = %s\n", __FUNCTION__, *argc, tok);
		argv[(*argc)++] = tok;
		tok = strtok(NULL, "\t ");
	}
}

/*
 * Add cheats for inserted game to cheat engine.
 */
static int activate_cheats(const char *boot2, const cheats_t *cheats, engine_t *engine)
{
	char elfname[FIO_PATH_MAX];
	enum dev dev = DEV_CD;
	char *argv[1];
	int argc = 1;
	game_t *game = NULL;
	cheat_t *cheat = NULL;
	code_t *code = NULL;
	gameid_t id;

	if (boot2 == NULL || (boot2 != NULL && (dev = get_dev(boot2)) == DEV_CD))
		_cdStandby(CDVD_BLOCK);

	if (boot2 == NULL) {
		if (cdGetElf(elfname) < 0) {
			A_PRINTF("Error: could not get ELF name from SYSTEM.CNF\n");
			_cdStop(CDVD_NOBLOCK);
			return -1;
		}
		boot2 = elfname;
	}

	get_argv(boot2, &argc, argv);

	if (gameid_generate(argv[0], &id) < 0) {
		A_PRINTF("Error: could not generate game ID from ELF %s\n", argv[0]);
		if (dev == DEV_CD)
			_cdStop(CDVD_NOBLOCK);
		return -1;
	}

	if (dev == DEV_CD)
		_cdStop(CDVD_NOBLOCK);

	if ((game = gameid_find(&id, &cheats->games)) == NULL) {
		A_PRINTF("Error: no cheats found for ELF %s\n", argv[0]);
		return -1;
	}

	/*
	 * Add hooks and codes for found game to cheat engine.
	 */
	engine_clear_hooks(engine);
	engine_clear_codes(engine);

	A_PRINTF("Activate cheats for \"%s\"\n", game->title);

	CHEATS_FOREACH(cheat, &game->cheats) {
		CODES_FOREACH(code, &cheat->codes) {
			D_PRINTF("%08X %08X\n", code->addr, code->val);
			/* TODO: improve check for hook */
			if ((code->addr & 0xfe000000) == 0x90000000)
				engine_add_hook(engine, code->addr, code->val);
			else
				engine_add_code(engine, code->addr, code->val);
		}
	}

	return 0;
}

/*
 * Start ELF specified by @boot2, or parse SYSTEM.CNF if @boot2 is NULL.
 */
static int start_game(const char *boot2)
{
	char elfname[FIO_PATH_MAX];
	enum dev dev = DEV_CD;
	char *argv[16];
	int argc = 16;

	if (boot2 == NULL || (boot2 != NULL && (dev = get_dev(boot2)) == DEV_CD))
		_cdStandby(CDVD_BLOCK);

	if (boot2 == NULL) {
		if (cdGetElf(elfname) < 0) {
			A_PRINTF("Error: could not get ELF name from SYSTEM.CNF\n");
			_cdStop(CDVD_NOBLOCK);
			return -1;
		}
		boot2 = elfname;
	}

	/* build args for LoadExecPS2() */
	get_argv(boot2, &argc, argv);

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
	padPortOpen(PAD_PORT, PAD_SLOT, g_padbuf);

	A_PRINTF("Error: could not load ELF %s\n", argv[0]);

	return -1;
}

int main(int argc, char *argv[])
{
	config_t config;
	cheats_t cheats;
	engine_t engine;
	const char *boot2 = NULL;
	int select = 0;

	SifInitRpc(0);
	init_scr();
	scr_clear();

	A_PRINTF(WELCOME_STRING);
	D_PRINTF("Build date: "APP_BUILD_DATE"\n");

	strcpy(g_bootpath, argv[0]);
	set_dir_name(g_bootpath);
	g_bootdev = get_dev(g_bootpath);
	A_PRINTF("Booting from: %s\n", g_bootpath);
	A_PRINTF("Initializing...\n");

	D_PRINTF("* Reading config...\n");
	_config_build(&config);
	if (config_read_file(&config, __pathname(CONFIG_FILE)) != CONFIG_TRUE)
		D_PRINTF("config: %s\n", config_error_text(&config));
	_config_print(&config);

	if (g_bootdev != DEV_HOST && _config_get_bool(&config, SET_IOP_RESET))
		reset_iop("rom0:UDNL rom0:EELOADCNF");

	if (_config_get_bool(&config, SET_SBV_PATCHES)) {
		D_PRINTF("* Applying SBV patches...\n");
		sbv_patch_enable_lmb();
		sbv_patch_disable_prefix_check();
		sbv_patch_user_mem_clear(0x00100000);
	}

	if (init_irx_modules(&config) < 0) {
		A_PRINTF("Error: failed to init IRX modules\n");
		goto end;
	}

	cdInit(CDVD_INIT_NOCHECK);
	_cdStop(CDVD_NOBLOCK);

	padInit(0);
	padPortOpen(PAD_PORT, PAD_SLOT, g_padbuf);
	padWaitReady(PAD_PORT, PAD_SLOT);
	padSetMainMode(PAD_PORT, PAD_SLOT, PAD_MMODE_DIGITAL, PAD_MMODE_LOCK);

	if (install_erls(&config, &engine) < 0) {
		A_PRINTF("Error: failed to install ERLs\n");
		goto end;
	}

	cheats_init(&cheats);
	load_cheats(&config, &cheats);

	A_PRINTF(OPTIONS);
	A_PRINTF("Ready.\n");

	/* Main loop */
	struct padButtonStatus btn;
	u32 old_pad = 0;
	for (;;) {
		padWaitReady(PAD_PORT, PAD_SLOT);
		if (!padRead(PAD_PORT, PAD_SLOT, &btn))
			continue;
		u32 paddata = 0xFFFF ^ btn.btns;
		u32 new_pad = paddata & ~old_pad;
		old_pad = paddata;

		if ((new_pad & PAD_START) || (new_pad & PAD_CROSS)) {
			start_game(boot2);
		} else if (new_pad & PAD_SELECT) {
			boot2 = _config_get_string_elem(&config, SET_BOOT2, select++);
			if (boot2 != NULL) {
				A_PRINTF("Boot ELF is %s\n", boot2);
			} else {
				A_PRINTF("Boot ELF is read from SYSTEM.CNF\n");
				select = 0;
			}
		} else if (new_pad & PAD_CIRCLE) {
			if (!_config_get_bool(&config, SET_ENGINE_INSTALL))
				A_PRINTF("Error: could not activate cheats - "
					"engine not installed\n");
			else
				activate_cheats(boot2, &cheats, &engine);
		} else if (new_pad & PAD_TRIANGLE) {
			/* Do nothing */
		} else if (new_pad & PAD_SQUARE) {
			/* Do nothing */
		} else if (new_pad & PAD_L1) {
			/* Do nothing */
		} else if (new_pad & PAD_L2) {
			/* Do nothing */
		} else if (new_pad & PAD_R1) {
			/* Do nothing */
		} else if (new_pad & PAD_R2) {
			/* Do nothing */
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
