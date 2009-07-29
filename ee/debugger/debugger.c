/*
 * debugger.c - EE side of remote debugger
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

/*
 * TODO: Eventually, this is where all the hacking magic happens:
 * - load our IOP modules on IOP reboot
 * - handle SIF RPC (send EE RAM to IOP etc.)
 * - manage code and hook list of cheat engine
 */

/*
 * This function is constantly called by the cheat engine.
 */
int debugger_main(int argc, char *argv[])
{
	return 0;
}
