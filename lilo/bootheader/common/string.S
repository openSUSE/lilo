/*  $Id$ */
/*
 * Copyright (C) Paul Mackerras 1997.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * NOTE: this code runs in 32 bit mode and is packaged as ELF32.
 */

#include <ppc_asm.h>

	.text
	.globl	strcpy
strcpy:
	addi	r5,r3,-1
	addi	r4,r4,-1
1:	lbzu	r0,1(r4)
	cmpwi	0,r0,0
	stbu	r0,1(r5)
	bne	1b
	blr

	.globl	strncpy
strncpy:
	cmpwi	0,r5,0
	beqlr
	mtctr	r5
	addi	r6,r3,-1
	addi	r4,r4,-1
1:	lbzu	r0,1(r4)
	cmpwi	0,r0,0
	stbu	r0,1(r6)
	bdnzf	2,1b		/* dec ctr, branch if ctr != 0 && !cr0.eq */
	blr

	.globl	strcat
strcat:
	addi	r5,r3,-1
	addi	r4,r4,-1
1:	lbzu	r0,1(r5)
	cmpwi	0,r0,0
	bne	1b
	addi	r5,r5,-1
1:	lbzu	r0,1(r4)
	cmpwi	0,r0,0
	stbu	r0,1(r5)
	bne	1b
	blr

	.globl	strcmp
strcmp:
	addi	r5,r3,-1
	addi	r4,r4,-1
1:	lbzu	r3,1(r5)
	cmpwi	1,r3,0
	lbzu	r0,1(r4)
	subf.	r3,r0,r3
	beqlr	1
	beq	1b
	blr

	.globl	strlen
strlen:
	addi	r4,r3,-1
1:	lbzu	r0,1(r4)
	cmpwi	0,r0,0
	bne	1b
	subf	r3,r3,r4
	blr

	.globl	memset
memset:
	rlwimi	r4,r4,8,16,23
	rlwimi	r4,r4,16,0,15
	addi	r6,r3,-4
	cmplwi	0,r5,4
	blt	7f
	stwu	r4,4(r6)
	beqlr
	andi.	r0,r6,3
	add	r5,r0,r5
	subf	r6,r0,r6
	rlwinm	r0,r5,32-2,2,31
	mtctr	r0
	bdz	6f
1:	stwu	r4,4(r6)
	bdnz	1b
6:	andi.	r5,r5,3
7:	cmpwi	0,r5,0
	beqlr
	mtctr	r5
	addi	r6,r6,3
8:	stbu	r4,1(r6)
	bdnz	8b
	blr

	.globl	memmove
memmove:
	cmpwi	0,r5,0
	beqlr
	cmplw	0,r3,r4
	bgt	backwards_memcpy
	/* fall through */

	.globl	memcpy
memcpy:
	cmpwi	0,r5,0
	beqlr
	andi.	r0,r4,3			/* get src word aligned */
	beq	20f
10:	subfic	r0,r0,4
	cmpd	r0,r5
	blt	11f
	mr	r0,r5
11:	mtctr	r0
12:	lbz	r7,0(r4)
	stb	r7,0(r3)
	addi	r4,r4,1
	addi	r3,r3,1
	bdnz	12b
	subf.	r5,r0,r5
	beqlr
20:	andi.	r0,r3,3			/* get dest word aligned */
	beq	30f
	subfic	r0,r0,4
	cmpd	r0,r5
	blt	21f
	mr	r0,r5
21:	mtctr	r0
22:	lbz	r7,0(r4)
	stb	r7,0(r3)
	addi	r4,r4,1
	addi	r3,r3,1
	bdnz	22b
	subf.	r5,r0,r5
	beqlr
	andi.	r0,r4,3			/* get src word aligned */
	bne	10b
30:
	rlwinm.	r7,r5,32-3,3,31		/* r7 = r5 >> 3 */
	addi	r6,r3,-4
	addi	r4,r4,-4
	beq	32f			/* if less than 8 bytes to do */
	mtctr	r7
31:	lwz	r7,4(r4)
	lwzu	r8,8(r4)
	stw	r7,4(r6)
	stwu	r8,8(r6)
	bdnz	31b
	andi.	r5,r5,7
32:	cmplwi	0,r5,4
	blt	33f
	lwzu	r0,4(r4)
	addi	r5,r5,-4
	stwu	r0,4(r6)
33:	cmpwi	0,r5,0
	beqlr
	mtctr	r5
	addi	r4,r4,3
	addi	r6,r6,3
34:	lbzu	r0,1(r4)
	stbu	r0,1(r6)
	bdnz	34b
	blr

	.globl	backwards_memcpy
backwards_memcpy:
	add	r6,r3,r5
	add	r4,r4,r5

	andi.	r0,r4,3			/* get src word aligned */
	beq	20f
10:	cmpd	r0,r5
	blt	11f
	mr	r0,r5
11:	mtctr	r0
12:	lbzu	r7,-1(r4)
	stbu	r7,-1(r6)
	bdnz	12b
	subf.	r5,r0,r5
	beqlr
20:	andi.	r0,r6,3			/* get dest word aligned */
	beq	30f
	cmpd	r0,r5
	blt	21f
	mr	r0,r5
21:	mtctr	r0
22:	lbzu	r7,-1(r4)
	stbu	r7,-1(r6)
	bdnz	22b
	subf.	r5,r0,r5
	beqlr
	andi.	r0,r4,3			/* get src word aligned */
	bne	10b
30:
	rlwinm.	r7,r5,32-3,3,31		/* r7 = r5 >> 3 */
	beq	32f
	mtctr	r7
31:	lwz	r7,-4(r4)
	lwzu	r8,-8(r4)
	stw	r7,-4(r6)
	stwu	r8,-8(r6)
	bdnz	31b
	andi.	r5,r5,7
32:	cmplwi	0,r5,4
	blt	33f
	lwzu	r0,-4(r4)
	subi	r5,r5,4
	stwu	r0,-4(r6)
33:	cmpwi	0,r5,0
	beqlr
	mtctr	r5
34:	lbzu	r0,-1(r4)
	stbu	r0,-1(r6)
	bdnz	34b
	blr

	.globl	memcmp
memcmp:
	cmpwi	0,r5,0
	blelr
	mtctr	r5
	addi	r6,r3,-1
	addi	r4,r4,-1
1:	lbzu	r3,1(r6)
	lbzu	r0,1(r4)
	subf.	r3,r0,r3
	bdnzt	2,1b
	blr


/*
 * Flush the dcache and invalidate the icache for a range of addresses.
 *
 * flush_cache(addr, len)
 */
	.global	flush_cache
flush_cache:
	addi	4,4,0x1f	/* len = (len + 0x1f) / 0x20 */
	rlwinm.	4,4,27,5,31
	mtctr	4
	beqlr
1:	dcbf	0,3
	icbi	0,3
	addi	3,3,0x20
	bdnz	1b
	sync
	isync
	blr

