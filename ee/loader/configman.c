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
	"engine.addr",
	"engine.file",
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

	/* build "loader" group */
	group = config_setting_add(root, "loader", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "iop_reset", CONFIG_TYPE_BOOL);
#ifdef _IOPRESET
	config_setting_set_bool(set, 1);
#endif
	set = config_setting_add(group, "sbv_patches", CONFIG_TYPE_BOOL);
#ifndef _NO_SBV
	config_setting_set_bool(set, 1);
#endif
	/* build "engine" group */
	group = config_setting_add(root, "engine", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "addr", CONFIG_TYPE_INT);
#ifdef ENGINE_ADDR
	config_setting_set_int(set, ENGINE_ADDR);
#endif
	set = config_setting_add(group, "file", CONFIG_TYPE_STRING);
#ifdef ENGINE_FILE
	config_setting_set_string(set, ENGINE_FILE);
#endif
	/* build "cheats" group */
	group = config_setting_add(root, "cheats", CONFIG_TYPE_GROUP);

	set = config_setting_add(group, "file", CONFIG_TYPE_STRING);
#ifdef CHEATS_FILE
	config_setting_set_string(set, CHEATS_FILE);
#endif
}

/**
 * config_print - Print out config.
 * @config: ptr to config
 */
void config_print(const config_t *config)
{
	u32 value;
	const char *s = NULL;

	if (config == NULL)
		return;

	printf("config values:\n");

	_config_lookup_bool(config, SET_IOP_RESET, (int*)&value);
	printf("%s = %i\n", setting_paths[SET_IOP_RESET], value);

	_config_lookup_bool(config, SET_SBV_PATCHES, (int*)&value);
	printf("%s = %i\n", setting_paths[SET_SBV_PATCHES], value);

	_config_lookup_u32(config, SET_ENGINE_ADDR, &value);
	printf("%s = %08x\n", setting_paths[SET_ENGINE_ADDR], value);

	_config_lookup_string(config, SET_ENGINE_FILE, &s);
	printf("%s = %s\n", setting_paths[SET_ENGINE_FILE], s);

	_config_lookup_string(config, SET_CHEATS_FILE, &s);
	printf("%s = %s\n", setting_paths[SET_CHEATS_FILE], s);
}
