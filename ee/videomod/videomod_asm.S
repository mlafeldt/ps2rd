/*
 * videomod_asm.S - Video mode patcher
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

	.text

	.set	push
	.set	noreorder
	.set	noat

	/*
	 * All global symbols are exported when built as ERL.
	 */
	.globl	HookSetGsCrt
	.globl	j_SetGsCrt

_start:

/*******************************************************************************
 *
 * HookSetGsCrt - Pre-call hook function for syscall SetGsCrt().
 *
 * This function is called every time before the syscall SetGsCrt() is executed.
 * It patches the 'pal_ntsc' arg ($a1) to force either PAL or NTSC.
 *
 ******************************************************************************/
	.ent	HookSetGsCrt
HookSetGsCrt:
	lw	$t0, videomode
	bnezl	$t0, j_SetGsCrt		/* don't patch if zero */
	move	$a1, $t0

	/* jump to original SetGsCrt() */
j_SetGsCrt:
	j	.
	nop
	.end	HookSetGsCrt

/*******************************************************************************
 *
 * .bss section
 *
 ******************************************************************************/
	.section .bss

videomode:
	.word	0
ydiff_lores:
	.word	0
ydiff_hires:
	.word	0

	.set	pop

/* EOF */
