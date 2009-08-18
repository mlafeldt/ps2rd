/*
 * configman.c - Configuration manager
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
#include <stdio.h>
#include <libconfig.h>
#include "configman.h"
#include "dbgprintf.h"

/* Paths to settings in config file */
static const char *setting_paths[] = {
	"loader.iop_reset",
	"loader.sbv_patches",
	"engine.install",
	"engine.addr",
	"engine.file",
	"debugger.install",
	"debugger.addr",
	"debugger.auto_hook",
	"debugger.rpc_mode",
	"sdklibs.install",
	"sdklibs.addr",
	"elfldr.install",
	"elfldr.addr",
	"cheats.file",
	NULL
};

/*
 * Some wrapper functions for libconfig.
 */
#define CONFIG_LOOKUP(config, key, value, type) \
{ \
	const char *path = setting_paths[key]; \
	int ret; \
	if (config == NULL || value == NULL) \
		return CONFIG_FALSE; \
	ret = config_lookup_##type(config, path, value); \
	if (ret != CONFIG_TRUE) \
		D_PRINTF("%s: setting %s not found\n", __FUNCTION__, path); \
	return ret; \
}

int _config_lookup_int(const config_t *config, enum setting_key key, long *value)
CONFIG_LOOKUP(config, key, value, int)

int _config_lookup_u32(const config_t *config, enum setting_key key, u32 *value)
CONFIG_LOOKUP(config, key, (long*)value, int)

#if 0
int _config_lookup_int64(const config_t *config, enum setting_key key, long long *value)
CONFIG_LOOKUP(config, key, value, int64)
#endif

int _config_lookup_float(const config_t *config, enum setting_key key, double *value)
CONFIG_LOOKUP(config, key, value, float)

int _config_lookup_bool(const config_t *config, enum setting_key key, int *value)
CONFIG_LOOKUP(config, key, value, bool)

int _config_lookup_string(const config_t *config, enum setting_key key, const char **value)
CONFIG_LOOKUP(config, key, value, string)


/* XXX create another macro for functions below? */
long config_get_int(const config_t *config, enum setting_key key)
{
	long value;

	_config_lookup_int(config, key, &value);

	return value;
}

u32 config_get_u32(const config_t *config, enum setting_key key)
{
	u32 value;

	_config_lookup_u32(config, key, &value);

	return value;
}

double config_get_float(const config_t *config, enum setting_key key)
{
	double value;

	_config_lookup_float(config, key, &value);

	return value;
}

int config_get_bool(const config_t *config, enum setting_key key)
{
	int value;

	_config_lookup_bool(config, key, &value);

	return value;
}

const char *config_get_string(const config_t *config, enum setting_key key)
{
	const char *s = NULL;

	_config_lookup_string(config, key, &s);

	/* The returned string must not be freed by the caller! */
	return s;
}


/**
 * config_build - Build configuration with all required settings.
 * @config: ptr to config
 */
void config_build(config_t *config)
{
	config_setting_t *root, *group, *set;

	if (config == NULL)
		return;

	config_init(config);
	root = config_root_setting(config);

	/*
	 * loader section
	 */
	group = config_setting_add(root, "loader", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "iop_reset", CONFIG_TYPE_BOOL);
#ifndef _NO_IOPRESET
	config_setting_set_bool(set, 1);
#endif
	set = config_setting_add(group, "sbv_patches", CONFIG_TYPE_BOOL);
#ifndef _NO_SBV
	config_setting_set_bool(set, 1);
#endif
	/*
	 * engine section
	 */
	group = config_setting_add(root, "engine", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "install", CONFIG_TYPE_BOOL);
#ifdef ENGINE_INSTALL
	config_setting_set_bool(set, 1);
#endif
	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef ENGINE_ADDR
	config_setting_set_int(set, ENGINE_ADDR);
#endif
	set = config_setting_add(group, "file", CONFIG_TYPE_STRING);
#ifdef ENGINE_FILE
	config_setting_set_string(set, ENGINE_FILE);
#endif
	/*
	 * debugger section
	 */
	group = config_setting_add(root, "debugger", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "install", CONFIG_TYPE_BOOL);
#ifdef DEBUGGER_INSTALL
	config_setting_set_bool(set, 1);
#endif
	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef DEBUGGER_ADDR
	config_setting_set_int(set, DEBUGGER_ADDR);
#endif
	set = config_setting_add(group, "auto_hook", CONFIG_TYPE_BOOL);
#ifdef DEBUGGER_AUTO_HOOK
	config_setting_set_bool(set, 1);
#endif
	set = config_setting_add(group, "rpc_mode", CONFIG_TYPE_INT);
#ifdef DEBUGGER_RPC_MODE
	config_setting_set_int(set, DEBUGGER_RPC_MODE);
#endif
	/*
	 * sdklibs section
	 */
	group = config_setting_add(root, "sdklibs", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "install", CONFIG_TYPE_BOOL);
#ifdef SDKLIBS_INSTALL
	config_setting_set_bool(set, 1);
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
	config_setting_set_bool(set, 1);
#endif
	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef ELFLDR_ADDR
	config_setting_set_int(set, ELFLDR_ADDR);
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
 * config_print - Print out all config settings.
 * @config: ptr to config
 */
void config_print(const config_t *config)
{
	u32 value;
	const char *s = NULL;

	printf("config values:\n");

#define PRINT_BOOL(key) \
	_config_lookup_bool(config, key, (int*)&value); \
	printf("%s = %i\n", setting_paths[key], value)
#define PRINT_INT(key) \
	_config_lookup_int(config, key, (long*)&value); \
	printf("%s = %i\n", setting_paths[key], value)
#define PRINT_U32(key) \
	_config_lookup_u32(config, key, &value); \
	printf("%s = %08x\n", setting_paths[key], value)
#define PRINT_STRING(key) \
	_config_lookup_string(config, key, &s); \
	printf("%s = %s\n", setting_paths[key], s)

	/* loader */
	PRINT_BOOL(SET_IOP_RESET);
	PRINT_BOOL(SET_SBV_PATCHES);

	/* engine */
	PRINT_BOOL(SET_ENGINE_INSTALL);
	PRINT_U32(SET_ENGINE_ADDR);
	PRINT_STRING(SET_ENGINE_FILE);

	/* debugger */
	PRINT_BOOL(SET_DEBUGGER_INSTALL);
	PRINT_U32(SET_DEBUGGER_ADDR);
	PRINT_BOOL(SET_DEBUGGER_AUTO_HOOK);
	PRINT_INT(SET_DEBUGGER_RPC_MODE);

	/* sdklibs */
	PRINT_BOOL(SET_SDKLIBS_INSTALL);
	PRINT_U32(SET_SDKLIBS_ADDR);

	/* elfldr */
	PRINT_BOOL(SET_ELFLDR_INSTALL);
	PRINT_U32(SET_ELFLDR_ADDR);

	/* cheats */
	PRINT_STRING(SET_CHEATS_FILE);
}
