/*
 * netlog_rpc.h - simple RPC interface to netlog.irx
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

#ifndef _EE_NETLOG_H_
#define _EE_NETLOG_H_

#define NETLOG_RPC_ID	0x0d0bfd40
#define NETLOG_MAX_MSG	1024

int netlog_init(const char *ipaddr, int ipport);
int netlog_exit(void);
int netlog_send(const char *format, ...);

#endif /* _EE_NETLOG_H_ */
