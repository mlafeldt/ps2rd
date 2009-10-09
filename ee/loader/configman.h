/*
 * configman.h - Configuration manager
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

#ifndef _CONFIGMAN_H_
#define _CONFIGMAN_H_

#include <tamtypes.h>
#include <libconfig.h>

/* Define some default settings */
#ifndef ENGINE_INSTALL
#define ENGINE_INSTALL		1
#endif
#ifndef ENGINE_ADDR
#define ENGINE_ADDR		0x00080000
#endif

#ifndef DEBUGGER_INSTALL
#define DEBUGGER_INSTALL	1
#endif
#ifndef DEBUGGER_ADDR
#define DEBUGGER_ADDR		0x00090000
#endif
#ifndef DEBUGGER_RPC_MODE
#define DEBUGGER_RPC_MODE	1
#endif
#ifndef DEBUGGER_LOAD_MODULES
#define DEBUGGER_LOAD_MODULES	1
#endif
#ifndef DEBUGGER_IPADDR
#define DEBUGGER_IPADDR		"192.168.0.10"
#endif
#ifndef DEBUGGER_NETMASK
#define DEBUGGER_NETMASK	"255.255.255.0"
#endif
#ifndef DEBUGGER_GATEWAY
#define DEBUGGER_GATEWAY	"192.168.0.1"
#endif

#ifndef SDKLIBS_INSTALL
#define SDKLIBS_INSTALL		1
#endif
#ifndef SDKLIBS_ADDR
#define SDKLIBS_ADDR		0x000c0000
#endif

#ifndef ELFLDR_INSTALL
#define ELFLDR_INSTALL		1
#endif
#ifndef ELFLDR_ADDR
#define ELFLDR_ADDR		0x000ff000
#endif

#ifndef VIDEOMOD_INSTALL
#define VIDEOMOD_INSTALL	0
#endif
#ifndef VIDEOMOD_ADDR
#define VIDEOMOD_ADDR		0x000f8000
#endif
#ifndef VIDEOMOD_VMODE
#define VIDEOMOD_VMODE		0
#endif

#ifndef CHEATS_FILE
#define CHEATS_FILE		"cheats.txt"
#endif

/* Keys to access different settings in configuration */
enum setting_key {
	/* loader */
	SET_IOP_RESET = 0,
	SET_SBV_PATCHES,
	SET_BOOT2,
	/* engine */
	SET_ENGINE_INSTALL,
	SET_ENGINE_ADDR,
	/* debugger */
	SET_DEBUGGER_INSTALL,
	SET_DEBUGGER_ADDR,
	SET_DEBUGGER_AUTO_HOOK,
	SET_DEBUGGER_PATCH_LOADMODULE,
	SET_DEBUGGER_RPC_MODE,
	SET_DEBUGGER_LOAD_MODULES,
	SET_DEBUGGER_IPADDR,
	SET_DEBUGGER_NETMASK,
	SET_DEBUGGER_GATEWAY,
	/* sdklibs */
	SET_SDKLIBS_INSTALL,
	SET_SDKLIBS_ADDR,
	/* elfldr */
	SET_ELFLDR_INSTALL,
	SET_ELFLDR_ADDR,
	/* videomod */
	SET_VIDEOMOD_INSTALL,
	SET_VIDEOMOD_ADDR,
	SET_VIDEOMOD_VMODE,
	/* cheats */
	SET_CHEATS_FILE
};

int _config_lookup_int(const config_t *config, enum setting_key key, long *value);
int _config_lookup_u32(const config_t *config, enum setting_key key, u32 *value);
int _config_lookup_float(const config_t *config, enum setting_key key, double *value);
int _config_lookup_bool(const config_t *config, enum setting_key key, int *value);
int _config_lookup_string(const config_t *config, enum setting_key key, const char **value);

long _config_get_int(const config_t *config, enum setting_key key);
u32 _config_get_u32(const config_t *config, enum setting_key key);
double _config_get_float(const config_t *config, enum setting_key key);
int _config_get_bool(const config_t *config, enum setting_key key);
const char *_config_get_string(const config_t *config, enum setting_key key);
const char *_config_get_string_elem(const config_t *config, enum setting_key key, int index);

int _config_setting_length(const config_t *config, enum setting_key key);

void _config_build(config_t *config);
void _config_print(const config_t *config);

#endif /* _CONFIGMAN_H_ */
