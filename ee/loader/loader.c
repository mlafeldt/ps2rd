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
#include <sifrpc.h>
#include <sbv_patches.h>
#include <string.h>
#include <libcheats.h>
#include <libconfig.h>
#include "configman.h"
#include "dbgprintf.h"
#include "engineman.h"
#include "mycdvd.h"
#include "mypad.h"
#include "mystring.h"
#include "myutil.h"

#define APP_NAME	"Artemis"
#define APP_VERSION	"0.2 WIP"
#define APP_BUILD_DATE	__DATE__" "__TIME__

#define WELCOME_STRING	"Welcome to "APP_NAME" "APP_VERSION"\n"

#define OPTIONS \
	"Options:\n" \
	"START | X - Start Game\n" \
	"CIRCLE    - Load Cheats\n"

#define PAD_PORT	0
#define PAD_SLOT	0

#ifndef CONFIG_FILE
#define CONFIG_FILE	"artemis.conf"
#endif


/* Boot information */
static char g_bootpath[FIO_PATH_MAX];
static enum bootdev g_bootdev = BOOT_UNKN;

/* IOP modules to load */
static const char *g_modules[] = {
	"rom0:SIO2MAN",
	"rom0:MCMAN",
	"rom0:MCSERV",
	"rom0:PADMAN",
	NULL
};

/* Built-in cheat engine (statically linked) */
extern u8 _binary_engine_erl_start[];
extern u8 _binary_engine_erl_end[];

/* Built-in debugger (statically linked) */
extern u8 _binary_debugger_erl_start[];
extern u8 _binary_debugger_erl_end[];


/*
 * Build pathname based on boot device and filename.
 */
static char *__pathname(const char *name)
{
	static char filename[FIO_PATH_MAX];
	enum bootdev dev;

	filename[0] = '\0';
	dev = get_bootdev(name);

	/* Add boot path if name is relative */
	if (dev == BOOT_UNKN)
		strcpy(filename, g_bootpath);

	strcat(filename, name);

	if (dev == BOOT_CD) {
		to_upper_str(filename);
		strcat(filename, ";1");
	}

	return filename;
}

/*
 * Install external or built-in engine.
 */
static int install_engine(const config_t *config, engine_ctx_t *ctx)
{
	const char *p = NULL;

	if (!config_get_bool(config, SET_ENGINE_INSTALL))
		return 0;

	if ((p = config_get_string(config, SET_ENGINE_FILE)) != NULL)
		return engine_install_from_file(__pathname(p),
			config_get_u32(config, SET_ENGINE_ADDR), ctx);
	else
		return engine_install_from_mem(_binary_engine_erl_start,
			config_get_u32(config, SET_ENGINE_ADDR), ctx);
}

/*
 * Load cheats from cheats file and pass them to the engine.
 */
static int load_cheats(const config_t *config, engine_ctx_t *ctx)
{
	const char *cheatfile = config_get_string(config, SET_CHEATS_FILE);
	char elfname[FIO_PATH_MAX];
	char *buf = NULL;
	cheats_t cheats;
	game_t *game = NULL;
	cheat_t *cheat = NULL;
	code_t *code = NULL;
	int found, ret;

	/*
	 * Read cheats from text file.
	 * TODO: this should be done only once or on demand.
	 */
	buf = read_text_file(__pathname(cheatfile), 0);
	if (buf == NULL) {
		A_PRINTF("Error: could not read cheats file '%s'\n", cheatfile);
		return -1;
	}

	cheats_init(&cheats);
	ret = cheats_read_buf(&cheats, buf);
	free(buf);
	if (ret != CHEATS_TRUE) {
		A_PRINTF("%s: line %i: %s\n", cheatfile, cheats.error_line,
			cheats.error_text);
		cheats_destroy(&cheats);
		return -1;
	}

	/*
	 * Get ELF filename of inserted game.
	 */
	cdDiskReady(CDVD_BLOCK);
	cdStandby();
	cdSync(CDVD_BLOCK);
	ret = cdGetElf(elfname);
	cdStop();
	cdSync(CDVD_BLOCK);
	if (ret < 0) {
		A_PRINTF("Error: could not get ELF name from SYSTEM.CNF\n");
		cheats_destroy(&cheats);
		return -1;
	}

	/*
	 * Search game list for title that includes the ELF name.
	 * TODO: use a real game ID instead of this crap.
	 */
	get_base_name(elfname, elfname);
	found = 0;
	GAMES_FOREACH(game, &cheats.games) {
		if (strstr(game->title, elfname) != NULL) {
			found = 1;
			break;
		}
	}

	if (!found) {
		A_PRINTF("Error: no cheats found for inserted game (%s)\n",
			elfname);
		cheats_destroy(&cheats);
		return -1;
	}

	/*
	 * Add hooks and codes for found game to cheat engine.
	 */
	if (!config_get_bool(config, SET_ENGINE_INSTALL)) {
		A_PRINTF("Error: cannot add cheats - engine not installed\n");
		cheats_destroy(&cheats);
		return -1;
	}

	engine_clear_hooks(ctx);
	engine_clear_codes(ctx);

	A_PRINTF("Loading cheats for \"%s\"\n", game->title);

	CHEATS_FOREACH(cheat, &game->cheats) {
		CODES_FOREACH(code, &cheat->codes) {
			D_PRINTF("%08X %08X\n", code->addr, code->val);
			/* TODO: improve check for hook */
			if ((code->addr & 0xfe000000) == 0x90000000)
				engine_add_hook(ctx, code->addr, code->val);
			else
				engine_add_code(ctx, code->addr, code->val);
		}
	}

	cheats_destroy(&cheats);

	return 0;
}

#include <erl.h>
/*
 * Install built-in debugger.
 * TODO: move debugger handling to a separate module
 */
static int install_debugger(const config_t *config, engine_ctx_t *ctx)
{
	struct erl_record_t *erl;
	struct symbol_t *sym;
	u32 addr = config_get_u32(config, SET_DEBUGGER_ADDR);
	void *debugger_main;

	if (!config_get_bool(config, SET_DEBUGGER_INSTALL))
		return 0;

	D_PRINTF("* Installing debugger...\n");
	D_PRINTF("ERL addr = %08x\n", addr);

	erl = load_erl_from_mem_to_addr(_binary_debugger_erl_start, addr, 0, NULL);
	if (erl == NULL) {
		D_PRINTF("%s: ERL load error\n", __FUNCTION__);
		return -1;
	}

	D_PRINTF("ERL size = %u\n", erl->fullsize);
	D_PRINTF("Install completed.\n");

#define GETSYM(x, s) \
	sym = erl_find_local_symbol(s, erl); \
	if (sym == NULL) { \
		D_PRINTF("%s: could not find symbol '%s'\n", __FUNCTION__, s); \
		return -2; \
	} \
	x = (typeof(x))sym->address; \
	D_PRINTF("%08x %s\n", sym->address, s)

	/* Set engine callback */
	GETSYM(debugger_main, "debugger_main");
	ctx->callbacks[0] = (u32)debugger_main;

	return 0;
}

int main(int argc, char *argv[])
{
	static u8 padbuf[256] __attribute__((aligned(64)));
	config_t config;
	engine_ctx_t ctx;
	int ret = 0;

	SifInitRpc(0);
	init_scr();
	scr_clear();

	A_PRINTF(WELCOME_STRING);
	D_PRINTF("Build date: "APP_BUILD_DATE"\n");

	strcpy(g_bootpath, argv[0]);
	set_dir_name(g_bootpath);
	g_bootdev = get_bootdev(g_bootpath);
	A_PRINTF("Booting from: %s\n", g_bootpath);
	A_PRINTF("Initializing...\n");

	D_PRINTF("* Reading config...\n");
	config_build(&config);
	if (config_read_file(&config, __pathname(CONFIG_FILE)) != CONFIG_TRUE)
		D_PRINTF("config: %s\n", config_error_text(&config));
	config_print(&config);

	if (config_get_bool(&config, SET_IOP_RESET))
		reset_iop("rom0:UDNL rom0:EELOADCNF");

	if (config_get_bool(&config, SET_SBV_PATCHES)) {
		D_PRINTF("* Applying SBV patches...\n");
		sbv_patch_enable_lmb();
		sbv_patch_disable_prefix_check();
		sbv_patch_user_mem_clear(0x00100000);
	}

	D_PRINTF("* Loading modules...\n");
	ret = load_modules(g_modules);
	if (ret < 0) {
		A_PRINTF("Error: failed to load IOP modules\n");
		goto end;
	}

	flush_caches();

	/* Init CDVD (non-blocking) */
	cdInit(CDVD_INIT_NOCHECK);
	cdDiskReady(CDVD_NOBLOCK);

	/* Init pad */
	padInit(0);
	padPortOpen(PAD_PORT, PAD_SLOT, padbuf);
	padWaitReady(PAD_PORT, PAD_SLOT);
	padSetMainMode(PAD_PORT, PAD_SLOT, PAD_MMODE_DIGITAL, PAD_MMODE_LOCK);

	/* Install ERL files */
	ret = install_engine(&config, &ctx);
	if (ret < 0) {
		A_PRINTF("Error: failed to install cheat engine\n");
		goto end;
	}
	ret = install_debugger(&config, &ctx);
	if (ret < 0) {
		A_PRINTF("Error: failed to install debugger\n");
		goto end;
	}

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
			A_PRINTF("Starting game...\n");
			if (cdRunElf() < 0)
				A_PRINTF("Error: could not load ELF\n");
		}

		if (new_pad & PAD_SELECT) {
			/* Do nothing */
		}

		if (new_pad & PAD_CIRCLE) {
			load_cheats(&config, &ctx);
		}

		if (new_pad & PAD_TRIANGLE) {
			/* Do nothing */
		}

		if (new_pad & PAD_SQUARE) {
			/* Do nothing */
		}
	}
end:
	A_PRINTF("Exit...\n");
	config_destroy(&config);
	SleepThread();

	return ret;
}
