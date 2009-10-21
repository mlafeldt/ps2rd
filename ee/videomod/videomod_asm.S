/*
 * videomod_asm.S - Video mode patcher
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 *
 * This file is part of ps2rd, the PS2 remote debugger.
 *
 * ps2rd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ps2rd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ps2rd.  If not, see <http://www.gnu.org/licenses/>.
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
	.globl	YPosHandler
	.globl	vmode
	.globl	ydiff_lores
	.globl	ydiff_hires

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
	lw	$t0, vmode
	bnezl	$t0, 1f			/* don't patch if zero */
	move	$a1, $t0
1:
	/* jump to original SetGsCrt() */
j_SetGsCrt:
	j	.
	nop
	.end	HookSetGsCrt

/*******************************************************************************
 *
 * YPosHandler - Fix y-position of screen for patched video mode.
 *
 * Actually, it's the debug exception handler that gets called every time the GS
 * registers DISPLAY1/2 are written to (data address breakpointing).
 *
 * Note: This function has been reverse-engineered from PS2 ZoneLoader.
 *
 ******************************************************************************/
	.ent	YPosHandler
YPosHandler:
	/* synchronize */
	sync.l
	sync.p

	/* save general purpose registers */
	sq	$k0, -0x10($zero)	/* mapped to 0x80080000-0x10 */
	la	$k0, savedregs
#if 0
	sq	$zero, 0($k0)
#endif
	sq	$at, 0x10($k0)
	sq	$v0, 0x20($k0)
	sq	$v1, 0x30($k0)
	sq	$a0, 0x40($k0)
	sq	$a1, 0x50($k0)
	sq	$a2, 0x60($k0)
	sq	$a3, 0x70($k0)
	sq	$t0, 0x80($k0)
	sq	$t1, 0x90($k0)
	sq	$t2, 0xa0($k0)
	sq	$t3, 0xb0($k0)
	sq	$t4, 0xc0($k0)
	sq	$t5, 0xd0($k0)
	sq	$t6, 0xe0($k0)
	sq	$t7, 0xf0($k0)
	sq	$s0, 0x100($k0)
	sq	$s1, 0x110($k0)
	sq	$s2, 0x120($k0)
	sq	$s3, 0x130($k0)
	sq	$s4, 0x140($k0)
	sq	$s5, 0x150($k0)
	sq	$s6, 0x160($k0)
	sq	$s7, 0x170($k0)
	sq	$t8, 0x180($k0)
	sq	$t9, 0x190($k0)
#if 0
	lq	$v0, -0x10($zero)
	sq	$v0, 0x1a0($k0)
#endif
	sq	$k1, 0x1b0($k0)
	sq	$gp, 0x1c0($k0)
	sq	$sp, 0x1d0($k0)
	sq	$fp, 0x1e0($k0)
	sq	$ra, 0x1f0($k0)

	/* decode store instruction at ErrorEPC (level 2 exception) */
	mfc0	$k0, $30
	.set	at
	sw	$k0, errorpc
	.set	noat
	lw	$t0, 0($k0)		/* t0 = opcode at ErrorEPC */
	srl	$t1, $t0, 16
	andi	$t1, $t1, 0x1f		/* t1 = rt */
	srl	$t3, $t0, 21
	andi	$t3, $t3, 0x1f		/* t3 = base */

	/* get address which triggered breakpoint */
	sll	$k0, $t3, 4		/* k0 = base * 16 */
	la	$t6, savedregs
	addu	$t6, $t6, $k0		/* t6 = &savedregs[base]; */
	lw	$t4, 0($t6)
	andi	$k1, $t0, 0xffff	/* k1 = offset */
	addu	$t4, $t4, $k1		/* t4 = savedregs[base] + offset; */

	/* get original DISPLAY1/2 value */
	sll	$k0, $t1, 4		/* k0 = rt * 16 */
	la	$t5, savedregs
	addu	$t5, $t5, $k0		/* t5 = &savedregs[rt]; */
	lq	$t2, 0($t5)		/* t2 = savedregs[rt]; */

	/* check if display is in low (DH < 257) or high res (DH >= 257) */
	dsrl32	$k0, $t2, 12
	andi	$k0, $k0, 0x7ff
	slti	$k1, $k0, 257
	beqz	$k1, hires
	nop
	la	$k0, ydiff_lores	/* low res */
	j	1f
	nop
hires:
	la	$k0, ydiff_hires
1:
	lw	$a0, 0($k0)		/* a0 = y offset (signed) */
	dsrl	$a1, $t2, 12
	andi	$a1, $a1, 0x7ff
	add	$a1, $a1, $a0		/* a1 = DY + offset */
	bgez	$a1, 2f
	nop
	move	$a1, $zero
2:
	/* patch DY and store modified value to GS register DISPLAY1/2 */
	li	$k0, 0xff800fff
	and	$k0, $t2, $k0
	sll	$a1, $a1, 12
	or	$t7, $k0, $a1
	sq	$t7, 0($t4)		/* XXX sq? */

	/*
	 * check if instruction is in a branch delay slot (BD2 bit in Cause
	 * register set?)
	 */
	mfc0	$k0, $13
	srl	$k0, 30
	andi	$k0, 1
	bnez	$k0, 3f
	nop

	/* if BD2 is 0, increment ErrorEPC and return from exception */
	mfc0	$k0, $30
	addiu	$k0, 4
	mtc0	$k0, $30
	j	y_ret
	nop
3:
	/* if BD2 is 1, decode opcode at ErrorEPC and decide what to do */
	mfc0	$k0, $30
	lw	$k0, 0($k0)

	/* if ((opcode & 0xfc1f07ff) == 9), opcode is jalr */
	li	$t0, 0xfc1f07ff
	and	$k1, $k0, $t0
	li	$t0, 9
	beq	$k1, $t0, 5f
	nop

	/* if ((opcode & 0xfc1ffff) == 8), opcode is jr */
	li	$t0, 0xfc1ffff
	and	$k1, $k0, $t0
	li	$t0, 8
	beq	$k1, $t0, 5f
	nop

	/* if ((opcode >> 26) == 2), opcode is j */
	srl	$k1, $k0, 26
	li	$t0, 2
	beq	$k1, $t0, 4f
	nop

	/* if ((opcode >> 26) == 3), opcode is jal */
	li	$t0, 3
	beq	$k1, $t0, 4f
	nop

	/*
	 * otherwise, opcode at ErrorEPC is a branch, therefore:
	 * ErrorEPC += 4 + ((opcode & 0xffff) << 2)
	 */
	li	$t0, 0xffff
	and	$k1, $k0, $t0
	sll	$k1, 2
	mfc0	$t0, $30
	addiu	$t0, 4
	addu	$t0, $k1
	mtc0	$t0, $30
	j	y_ret
	nop
4:
	/*
	 * opcode at ErrorEPC is either j or jal, so get the target address and
	 * move it into ErrorEPC
	 */
	li	$t0, 0x03ffffff
	and	$k1, $k0, $t0
	sll	$k1, 2
	mfc0	$t0, $30
	addiu	$t0, 4
	lui	$t1, 0xf000
	and	$t0, $t1
	addu	$t0, $k1
	mtc0	$t0, $30
	j	y_ret
	nop
5:
	/*
	 * opcode at ErrorEPC is either jalr or jr, so get the target address
	 * and move it into ErrorEPC
	 */
	srl	$k1, $k0, 21
	andi	$k1, 0x1f
	sll	$t1, $k1, 4
	la	$t0, savedregs
	addu	$t0, $t1
	lw	$t0, 0($t0)
	mtc0	$t0, $30
y_ret:
	/* restore general purpose registers */
	la	$k0, savedregs
	lq	$ra, 0x1f0($k0)
	lq	$fp, 0x1e0($k0)
	lq	$sp, 0x1d0($k0)
	lq	$gp, 0x1c0($k0)
	lq	$k1, 0x1b0($k0)
	lq	$t9, 0x190($k0)
	lq	$t8, 0x180($k0)
	lq	$s7, 0x170($k0)
	lq	$s6, 0x160($k0)
	lq	$s5, 0x150($k0)
	lq	$s4, 0x140($k0)
	lq	$s3, 0x130($k0)
	lq	$s2, 0x120($k0)
	lq	$s1, 0x110($k0)
	lq	$s0, 0x100($k0)
	lq	$t7, 0xf0($k0)
	lq	$t6, 0xe0($k0)
	lq	$t5, 0xd0($k0)
	lq	$t4, 0xc0($k0)
	lq	$t3, 0xb0($k0)
	lq	$t2, 0xa0($k0)
	lq	$t1, 0x90($k0)
	lq	$t0, 0x80($k0)
	lq	$a3, 0x70($k0)
	lq	$a2, 0x60($k0)
	lq	$a1, 0x50($k0)
	lq	$a0, 0x40($k0)
	lq	$v1, 0x30($k0)
	lq	$v0, 0x20($k0)
	lq	$at, 0x10($k0)
	lq	$k0, -0x10($zero)

	/* synchronize */
	sync.p
	sync.l

	/* return from exception */
	eret
.end	YPosHandler

/*******************************************************************************
 *
 * .bss section
 *
 ******************************************************************************/
	.section .bss

vmode:
	.word	0
ydiff_lores:
	.word	0
ydiff_hires:
	.word	0

errorpc:
	.word	0

	.p2align 4
savedregs:
	.space	(32*16)

	.set	pop

/* EOF */
