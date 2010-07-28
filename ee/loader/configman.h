/*
 * Configuration manager
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

#ifndef _CONFIGMAN_H_
#define _CONFIGMAN_H_

#include <tamtypes.h>
#include <libconfig.h>

#ifndef CONFIG_FILE
#define CONFIG_FILE		"ps2rd.conf"
#endif

/* Define some default settings */
#ifndef IOP_RESET
#define IOP_RESET		1
#endif
#ifndef SBV_PATCHES
#define SBV_PATCHES		1
#endif
#ifndef USB_SUPPORT
#define USB_SUPPORT		1
#endif
#ifndef CHEATS_FILE
#define CHEATS_FILE		"cheats.txt"
#endif

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
#ifndef DEBUGGER_SMS_MODULES
#define DEBUGGER_SMS_MODULES	1
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
#define VIDEOMOD_VMODE		2
#endif
#ifndef VIDEOMOD_YFIX
#define VIDEOMOD_YFIX		1
#endif
#ifndef VIDEOMOD_YDIFF_LORES
#define VIDEOMOD_YDIFF_LORES	0
#endif
#ifndef VIDEOMOD_YDIFF_HIRES
#define VIDEOMOD_YDIFF_HIRES	0
#endif


/* Keys to access different settings in configuration */
#define SET_IOP_RESET                   "loader.iop_reset"
#define SET_SBV_PATCHES                 "loader.sbv_patches"
#define SET_USB_SUPPORT                 "loader.usb_support"
#define SET_BOOT2                       "loader.boot2"
#define SET_CHEATS_FILE                 "loader.cheats"
#define SET_ENGINE_INSTALL              "engine.install"
#define SET_ENGINE_ADDR                 "engine.addr"
#define SET_DEBUGGER_INSTALL            "debugger.install"
#define SET_DEBUGGER_ADDR               "debugger.addr"
#define SET_DEBUGGER_AUTO_HOOK          "debugger.auto_hook"
#define SET_DEBUGGER_PATCH_LOADMODULE	"debugger.patch_loadmodule"
#define SET_DEBUGGER_UNHOOK_IOP_RESET	"debugger.unhook_iop_reset"
#define SET_DEBUGGER_RPC_MODE           "debugger.rpc_mode"
#define SET_DEBUGGER_LOAD_MODULES	"debugger.load_modules"
#define SET_DEBUGGER_SMS_MODULES	"debugger.sms_modules"
#define SET_DEBUGGER_IPADDR             "debugger.ipaddr"
#define SET_DEBUGGER_NETMASK            "debugger.netmask"
#define SET_DEBUGGER_GATEWAY            "debugger.gateway"
#define SET_SDKLIBS_INSTALL             "sdklibs.install"
#define SET_SDKLIBS_ADDR                "sdklibs.addr"
#define SET_ELFLDR_INSTALL              "elfldr.install"
#define SET_ELFLDR_ADDR                 "elfldr.addr"
#define SET_VIDEOMOD_INSTALL            "videomod.install"
#define SET_VIDEOMOD_ADDR               "videomod.addr"
#define SET_VIDEOMOD_VMODE              "videomod.vmode"
#define SET_VIDEOMOD_YFIX               "videomod.yfix"
#define SET_VIDEOMOD_YDIFF_LORES	"videomod.ydiff_lores"
#define SET_VIDEOMOD_YDIFF_HIRES	"videomod.ydiff_hires"


void config_build(config_t *config);
void config_print(const config_t *config);

/*
 * libconfig wrapper functions for lazy people.
 */

static inline int config_get_int(const config_t *config, const char *path)
{
	return config_setting_get_int(config_lookup(config, path));
}

static inline long long config_get_int64(const config_t *config, const char *path)
{
	return config_setting_get_int64(config_lookup(config, path));
}

static inline double config_get_float(const config_t *config, const char *path)
{
	return config_setting_get_float(config_lookup(config, path));
}

static inline int config_get_bool(const config_t *config, const char *path)
{
	return config_setting_get_bool(config_lookup(config, path));
}

static inline const char *config_get_string(const config_t *config, const char *path)
{
	return config_setting_get_string(config_lookup(config, path));
}

static inline const char *config_get_string_elem(const config_t *config, const char *path, int index)
{
	return config_setting_get_string_elem(config_lookup(config, path), index);
}

#endif /* _CONFIGMAN_H_ */
