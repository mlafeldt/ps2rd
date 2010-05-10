/*
 * loadfile.c - EE side of remote debugger
 *
 * Copyright (C) 2009-2010 jimmikaelkael <jimmikaelkael@wanadoo.fr>
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
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>
#include <ps2lib_err.h>
#include <loadfile.h>

struct lf_module_load_arg {
	union {
		int arg_len;
		int	result;
	} p;
	int  modres;
	char path[LF_PATH_MAX];
	char args[LF_ARG_MAX];
} __attribute__((aligned (16)));


#define LF_F_MOD_LOAD 0

/* from libkernel */
extern SifRpcClientData_t _lf_cd;
extern int _lf_init;

/*
 * SifLoadModuleAsync - load module asynchronously (non-blocking rpc)
 */
int SifLoadModuleAsync(const char *path, int arg_len, const char *args)
{
	struct lf_module_load_arg arg;

	if (SifLoadFileInit() < 0)
		return -E_LIB_API_INIT;

	memset(&arg, 0, sizeof arg);

	strncpy(arg.path, path, LF_PATH_MAX - 1);
	arg.path[LF_PATH_MAX - 1] = 0;

	if ((args) && (arg_len)) {
		arg.p.arg_len = arg_len > LF_ARG_MAX ? LF_ARG_MAX : arg_len;
		memcpy(arg.args, args, arg.p.arg_len);
	}
	else arg.p.arg_len = 0;

	if (SifCallRpc(&_lf_cd, LF_F_MOD_LOAD, SIF_RPC_M_NOWAIT, &arg, sizeof(arg), &arg, 8, NULL, NULL) < 0)
		return -E_SIF_RPC_CALL;

	return 0;
}
