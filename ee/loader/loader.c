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
#include <loadfile.h>
#include <sbv_patches.h>
#include <sifrpc.h>
#include <string.h>
#include <libcheats.h>
#include <libconfig.h>
#include "configman.h"
#include "dbgprintf.h"
#include "engineman.h"
#include "erlman.h"
#include "mycdvd.h"
#include "mypad.h"
#include "mystring.h"
#include "myutil.h"
#include "netlog_rpc.h"

#define APP_NAME	"Artemis"
#define APP_VERSION	"0.2"
#define APP_BUILD_DATE	__DATE__" "__TIME__

#define WELCOME_STRING	"Welcome to "APP_NAME" "APP_VERSION"\n"

#define OPTIONS \
	"Options:\n" \
	"START | X - Start Game\n" \
	"CIRCLE    - Activate Cheats\n"

#define PAD_PORT	0
#define PAD_SLOT	0

#ifndef CONFIG_FILE
#define CONFIG_FILE	"artemis.conf"
#endif

#define NETLOG_IP	"192.168.0.2"
#define NETLOG_PORT	7411

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

/* Statically linked IRX files */
extern u8  _ps2dev9_irx_start[];
extern u8  _ps2dev9_irx_end[];
extern int _ps2dev9_irx_size;
extern u8  _ps2ip_irx_start[];
extern u8  _ps2ip_irx_end[];
extern int _ps2ip_irx_size;
extern u8  _ps2smap_irx_start[];
extern u8  _ps2smap_irx_end[];
extern int _ps2smap_irx_size;
extern u8  _debugger_irx_start[];
extern u8  _debugger_irx_end[];
extern int _debugger_irx_size;
extern u8  _netlog_irx_start[];
extern u8  _netlog_irx_end[];
extern int _netlog_irx_size;

/* TODO: make it configurable */
#define IRX_ADDR	0x80030000

#define IRX_NUM		5

/* RAM file table entry */
typedef struct {
	u32	hash;
	u8	*addr;
	u32	size;
} ramfile_t;

/**
 * strhash - String hashing function as specified by the ELF ABI.
 * @name: string to calculate hash from
 * @return: 32-bit hash value
 */
static u32 strhash(const char *name)
{
	const u8 *p = (u8*)name;
	u32 h = 0, g;

	while (*p) {
		h = (h << 4) + *p++;
		if ((g = (h & 0xf0000000)) != 0)
			h ^= (g >> 24);
		h &= ~g;
	}

	return h;
}

/*
 * Helper to populate an RAM file table entry.
 */
static void ramfile_set(ramfile_t *file, const char *name, u8 *addr, u32 size)
{
	file->hash = name ? strhash(name) : 0;
	file->addr = addr;
	file->size = size;

	D_PRINTF("%s: name=%s hash=%08x addr=%08x size=%i\n", __FUNCTION__,
		name, file->hash, (u32)file->addr, file->size);
}

/*
 * Copy statically linked IRX files to kernel RAM.
 * They will be loaded by the debugger later...
 */
static void copy_modules_to_kernel(u32 addr)
{
	ramfile_t file_tab[IRX_NUM + 1];
	ramfile_t *file_ptr = file_tab;
	ramfile_t *ktab = NULL;

	D_PRINTF("%s: addr=%08x\n", __FUNCTION__, addr);

	/*
	 * build RAM file table
	 */
	ramfile_set(file_ptr++, "ps2dev9", _ps2dev9_irx_start, _ps2dev9_irx_size);
	ramfile_set(file_ptr++, "ps2ip", _ps2ip_irx_start, _ps2ip_irx_size);
	ramfile_set(file_ptr++, "ps2smap", _ps2smap_irx_start, _ps2smap_irx_size);
	ramfile_set(file_ptr++, "debugger", _debugger_irx_start, _debugger_irx_size);
	ramfile_set(file_ptr++, "netlog", _netlog_irx_start, _netlog_irx_size);
	ramfile_set(file_ptr, NULL, 0, 0); /* terminator */

	/*
	 * copy modules to kernel RAM
	 *
	 * memory structure at @addr:
	 * |RAM file table|IRX module #1|IRX module #2|etc.
	 */
	DI();
	ee_kmode_enter();

	ktab = (ramfile_t*)addr;
	addr += sizeof(file_tab);
	file_ptr = file_tab;

	while (file_ptr->hash) {
		memcpy((u8*)addr, file_ptr->addr, file_ptr->size);
		file_ptr->addr = (u8*)addr;
		addr += file_ptr->size;
		file_ptr++;
	}

	memcpy(ktab, file_tab, sizeof(file_tab));

	ee_kmode_exit();
	EI();

	FlushCache(0);
}

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

extern u8  _engine_erl_start[];
extern u8  _engine_erl_end[];
extern int _engine_erl_size;

/*
 * Install external or built-in engine.
 */
static int install_engine(const config_t *config, engine_t *engine)
{
	const char *p = NULL;
	u32 addr;

	if (!config_get_bool(config, SET_ENGINE_INSTALL))
		return 0;

	addr = config_get_u32(config, SET_ENGINE_ADDR);

	if ((p = config_get_string(config, SET_ENGINE_FILE)) != NULL)
		return engine_install_from_file(__pathname(p), addr, engine);
	else
		return engine_install_from_mem(_engine_erl_start, addr, engine);
}

/*
 * Load cheats from text file.
 */
static int load_cheats(const config_t *config, cheats_t *cheats)
{
	const char *cheatfile = config_get_string(config, SET_CHEATS_FILE);
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
 * Add cheats for inserted game to cheat engine.
 */
static int activate_cheats(const cheats_t *cheats, engine_t *engine)
{
	char elfname[FIO_PATH_MAX];
	game_t *game = NULL;
	cheat_t *cheat = NULL;
	code_t *code = NULL;
	int found, ret;

	/*
	 * Get ELF filename of inserted game.
	 */
	cdDiskReady(CDVD_BLOCK);
	cdStandby();
	cdSync(CDVD_BLOCK);
	ret = cdGetElf(elfname);
	cdStop();
	cdSync(CDVD_NOBLOCK);
	if (ret < 0) {
		A_PRINTF("Error: could not get ELF name from SYSTEM.CNF\n");
		return -1;
	}

	/*
	 * Search game list for title that includes the ELF name.
	 * TODO: use a real game ID instead of this crap.
	 */
	get_base_name(elfname, elfname);
	found = 0;
	GAMES_FOREACH(game, &cheats->games) {
		if (strstr(game->title, elfname) != NULL) {
			found = 1;
			break;
		}
	}

	if (!found) {
		A_PRINTF("Error: no cheats found for inserted game (%s)\n",
			elfname);
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

extern void *_ps2sdk_libc_deinit();

int main(int argc, char *argv[])
{
	static u8 padbuf[256] __attribute__((aligned(64)));
	config_t config;
	cheats_t cheats;
	engine_t engine;
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

	if (g_bootdev != BOOT_HOST && config_get_bool(&config, SET_IOP_RESET))
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
#if 0
	SifExecModuleBuffer(_netlog_irx_start, _netlog_irx_size, 0, NULL, &ret);
	netlog_init(NETLOG_IP, NETLOG_PORT);
#endif
	copy_modules_to_kernel(IRX_ADDR);

	/* Install ERL files */
	ret = install_engine(&config, &engine);
	if (ret < 0) {
		A_PRINTF("Error: failed to install cheat engine\n");
		goto end;
	}
	ret = install_erls(&config, &engine);
	if (ret < 0) {
		A_PRINTF("Error: failed to install ERLs\n");
		goto end;
	}

	/* Init CDVD (non-blocking) */
	cdInit(CDVD_INIT_NOCHECK);
	cdDiskReady(CDVD_NOBLOCK);
	cdStop();
	cdSync(CDVD_NOBLOCK);

	/* Init pad */
	padInit(0);
	padPortOpen(PAD_PORT, PAD_SLOT, padbuf);
	padWaitReady(PAD_PORT, PAD_SLOT);
	padSetMainMode(PAD_PORT, PAD_SLOT, PAD_MMODE_DIGITAL, PAD_MMODE_LOCK);

	/* Load cheats */
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
			A_PRINTF("Starting game...\n");
			padPortClose(PAD_PORT, PAD_SLOT);
			padReset();
			//_ps2sdk_libc_deinit();
			if (cdRunElf() < 0)
				A_PRINTF("Error: could not load ELF\n");
		}

		if (new_pad & PAD_SELECT) {
			/* Do nothing */
		}

		if (new_pad & PAD_CIRCLE) {
			if (!config_get_bool(&config, SET_ENGINE_INSTALL))
				A_PRINTF("Error: could not activate cheats - "
					"engine not installed\n");
			else
				activate_cheats(&cheats, &engine);
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
