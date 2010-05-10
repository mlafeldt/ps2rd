/*
 * configman.c - Configuration manager
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
 */

#include <tamtypes.h>
#include <stdio.h>
#include <libconfig.h>
#include "configman.h"
#include "dbgprintf.h"

/**
 * _config_build - Build configuration with all required settings.
 * @config: ptr to config
 */
void _config_build(config_t *config)
{
	config_setting_t *root, *group, *set;

	config_init(config);
	root = config_root_setting(config);

	/*
	 * loader section
	 */
	group = config_setting_add(root, "loader", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "iop_reset", CONFIG_TYPE_BOOL);
#ifdef IOP_RESET
	config_setting_set_bool(set, IOP_RESET);
#endif
	set = config_setting_add(group, "sbv_patches", CONFIG_TYPE_BOOL);
#ifdef SBV_PATCHES
	config_setting_set_bool(set, SBV_PATCHES);
#endif
	set = config_setting_add(group, "usb_support", CONFIG_TYPE_BOOL);
#ifdef USB_SUPPORT
	config_setting_set_bool(set, USB_SUPPORT);
#endif
	set = config_setting_add(group, "boot2", CONFIG_TYPE_ARRAY);

	/*
	 * engine section
	 */
	group = config_setting_add(root, "engine", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "install", CONFIG_TYPE_BOOL);
#ifdef ENGINE_INSTALL
	config_setting_set_bool(set, ENGINE_INSTALL);
#endif
	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef ENGINE_ADDR
	config_setting_set_int(set, ENGINE_ADDR);
#endif
	/*
	 * debugger section
	 */
	group = config_setting_add(root, "debugger", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "install", CONFIG_TYPE_BOOL);
#ifdef DEBUGGER_INSTALL
	config_setting_set_bool(set, DEBUGGER_INSTALL);
#endif
	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef DEBUGGER_ADDR
	config_setting_set_int(set, DEBUGGER_ADDR);
#endif
	set = config_setting_add(group, "auto_hook", CONFIG_TYPE_BOOL);
#ifdef DEBUGGER_AUTO_HOOK
	config_setting_set_bool(set, DEBUGGER_AUTO_HOOK);
#endif
	set = config_setting_add(group, "patch_loadmodule", CONFIG_TYPE_BOOL);
#ifdef DEBUGGER_PATCH_LOADMODULE
	config_setting_set_bool(set, DEBUGGER_PATCH_LOADMODULE);
#endif
	set = config_setting_add(group, "unhook_iop_reset", CONFIG_TYPE_BOOL);
#ifdef DEBUGGER_UNHOOK_IOP_RESET
	config_setting_set_bool(set, DEBUGGER_UNHOOK_IOP_RESET);
#endif
	set = config_setting_add(group, "rpc_mode", CONFIG_TYPE_INT);
#ifdef DEBUGGER_RPC_MODE
	config_setting_set_int(set, DEBUGGER_RPC_MODE);
#endif
	set = config_setting_add(group, "load_modules", CONFIG_TYPE_BOOL);
#ifdef DEBUGGER_LOAD_MODULES
	config_setting_set_bool(set, DEBUGGER_LOAD_MODULES);
#endif
	set = config_setting_add(group, "sms_modules", CONFIG_TYPE_BOOL);
#ifdef DEBUGGER_SMS_MODULES
	config_setting_set_bool(set, DEBUGGER_SMS_MODULES);
#endif
	set = config_setting_add(group, "ipaddr", CONFIG_TYPE_STRING);
#ifdef DEBUGGER_IPADDR
	config_setting_set_string(set, DEBUGGER_IPADDR);
#endif
	set = config_setting_add(group, "netmask", CONFIG_TYPE_STRING);
#ifdef DEBUGGER_NETMASK
	config_setting_set_string(set, DEBUGGER_NETMASK);
#endif
	set = config_setting_add(group, "gateway", CONFIG_TYPE_STRING);
#ifdef DEBUGGER_GATEWAY
	config_setting_set_string(set, DEBUGGER_GATEWAY);
#endif
	/*
	 * sdklibs section
	 */
	group = config_setting_add(root, "sdklibs", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "install", CONFIG_TYPE_BOOL);
#ifdef SDKLIBS_INSTALL
	config_setting_set_bool(set, SDKLIBS_INSTALL);
#endif
	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef SDKLIBS_ADDR
	config_setting_set_int(set, SDKLIBS_ADDR);
#endif
	/*
	 * elfldr section
	 */
	group = config_setting_add(root, "elfldr", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "install", CONFIG_TYPE_BOOL);
#ifdef ELFLDR_INSTALL
	config_setting_set_bool(set, ELFLDR_INSTALL);
#endif
	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef ELFLDR_ADDR
	config_setting_set_int(set, ELFLDR_ADDR);
#endif
	/*
	 * videomod section
	 */
	group = config_setting_add(root, "videomod", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "install", CONFIG_TYPE_BOOL);
#ifdef VIDEOMOD_INSTALL
	config_setting_set_bool(set, VIDEOMOD_INSTALL);
#endif
	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef VIDEOMOD_ADDR
	config_setting_set_int(set, VIDEOMOD_ADDR);
#endif
	set = config_setting_add(group, "vmode", CONFIG_TYPE_INT);
#ifdef VIDEOMOD_VMODE
	config_setting_set_int(set, VIDEOMOD_VMODE);
#endif
	set = config_setting_add(group, "yfix", CONFIG_TYPE_BOOL);
#ifdef VIDEOMOD_YFIX
	config_setting_set_bool(set, VIDEOMOD_YFIX);
#endif
	set = config_setting_add(group, "ydiff_lores", CONFIG_TYPE_INT);
#ifdef VIDEOMOD_YDIFF_LORES
	config_setting_set_int(set, VIDEOMOD_YDIFF_LORES);
#endif
	set = config_setting_add(group, "ydiff_hires", CONFIG_TYPE_INT);
#ifdef VIDEOMOD_YDIFF_HIRES
	config_setting_set_int(set, VIDEOMOD_YDIFF_HIRES);
#endif
	/*
	 * cheats section
	 */
	group = config_setting_add(root, "cheats", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "file", CONFIG_TYPE_STRING);
#ifdef CHEATS_FILE
	config_setting_set_string(set, CHEATS_FILE);
#endif
}

/**
 * _config_print - Print out all config settings.
 * @config: ptr to config
 */
void _config_print(const config_t *config)
{
	const char *s = NULL;
	int i;

	printf("config settings:\n");

#define PRINT_BOOL(key) \
	printf("%s = %i\n", key, _config_get_bool(config, key))
#define PRINT_INT(key) \
	printf("%s = %i\n", key, _config_get_int(config, key))
#define PRINT_U32(key) \
	printf("%s = %08x\n", key, _config_get_int(config, key))
#define PRINT_STRING(key) \
	printf("%s = %s\n", key, _config_get_string(config, key))
#define PRINT_STRING_ARRAY(key) \
	i = 0; \
	do { \
		s = _config_get_string_elem(config, key, i); \
		printf("%s[%i] = %s\n", key, i, s); \
		i++; \
	} while (s != NULL)

	/* loader */
	PRINT_BOOL(SET_IOP_RESET);
	PRINT_BOOL(SET_SBV_PATCHES);
	PRINT_BOOL(SET_USB_SUPPORT);
	PRINT_STRING_ARRAY(SET_BOOT2);

	/* engine */
	PRINT_BOOL(SET_ENGINE_INSTALL);
	PRINT_U32(SET_ENGINE_ADDR);

	/* debugger */
	PRINT_BOOL(SET_DEBUGGER_INSTALL);
	PRINT_U32(SET_DEBUGGER_ADDR);
	PRINT_BOOL(SET_DEBUGGER_AUTO_HOOK);
	PRINT_BOOL(SET_DEBUGGER_PATCH_LOADMODULE);
	PRINT_BOOL(SET_DEBUGGER_UNHOOK_IOP_RESET);
	PRINT_INT(SET_DEBUGGER_RPC_MODE);
	PRINT_BOOL(SET_DEBUGGER_LOAD_MODULES);
	PRINT_BOOL(SET_DEBUGGER_SMS_MODULES);
	PRINT_STRING(SET_DEBUGGER_IPADDR);
	PRINT_STRING(SET_DEBUGGER_NETMASK);
	PRINT_STRING(SET_DEBUGGER_GATEWAY);

	/* sdklibs */
	PRINT_BOOL(SET_SDKLIBS_INSTALL);
	PRINT_U32(SET_SDKLIBS_ADDR);

	/* elfldr */
	PRINT_BOOL(SET_ELFLDR_INSTALL);
	PRINT_U32(SET_ELFLDR_ADDR);

	/* videomod */
	PRINT_BOOL(SET_VIDEOMOD_INSTALL);
	PRINT_U32(SET_VIDEOMOD_ADDR);
	PRINT_INT(SET_VIDEOMOD_VMODE);
	PRINT_BOOL(SET_VIDEOMOD_YFIX);
	PRINT_INT(SET_VIDEOMOD_YDIFF_LORES);
	PRINT_INT(SET_VIDEOMOD_YDIFF_HIRES);

	/* cheats */
	PRINT_STRING(SET_CHEATS_FILE);
}
