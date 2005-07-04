/*
 * arch/ppc/boot/common/util.S
 *
 * Useful bootup functions, which are more easily done in asm than C.
 *
 * NOTE:  Be very very careful about the registers you use here.
 *	We don't follow any ABI calling convention among the
 *	assembler functions that call each other, especially early
 *	in the initialization.  Please preserve at least r3 and r4
 *	for these early functions, as they often contain information
 *	passed from boot roms into the C decompress function.
 *
 * Author: Tom Rini
 *	   trini@mvista.com
 * Derived from arch/ppc/boot/prep/head.S (Cort Dougan, many others).
 *
 * 2001-2004 (c) MontaVista, Software, Inc.  This file is licensed under
 * the terms of the GNU General Public License version 2.  This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

#include <ppc_asm.h>
#include <ppc-reg.h>


	.text

	.globl	disable_6xx_mmu
disable_6xx_mmu:
	/* Establish default MSR value, exception prefix 0xFFF.
	 * If necessary, this function must fix up the LR if we
	 * return to a different address space once the MMU is
	 * disabled.
	 */
	li	r8,MSR_IP|MSR_FP
	mtmsr	r8
	isync

	/* Test for a 601 */
	mfpvr	r10
	srwi	r10,r10,16
	cmpwi	0,r10,1		/* 601 ? */
	beq	.clearbats_601

	/* Clear BATs */
	li	r8,0
	mtspr	SPRN_DBAT0U,r8
	mtspr	SPRN_DBAT0L,r8
	mtspr	SPRN_DBAT1U,r8
	mtspr	SPRN_DBAT1L,r8
	mtspr	SPRN_DBAT2U,r8
	mtspr	SPRN_DBAT2L,r8
	mtspr	SPRN_DBAT3U,r8
	mtspr	SPRN_DBAT3L,r8
.clearbats_601:
	mtspr	SPRN_IBAT0U,r8
	mtspr	SPRN_IBAT0L,r8
	mtspr	SPRN_IBAT1U,r8
	mtspr	SPRN_IBAT1L,r8
	mtspr	SPRN_IBAT2U,r8
	mtspr	SPRN_IBAT2L,r8
	mtspr	SPRN_IBAT3U,r8
	mtspr	SPRN_IBAT3L,r8
	isync
	sync
	sync

	/* Set segment registers */
	li	r8,16		/* load up segment register values */
	mtctr	r8		/* for context 0 */
	lis	r8,0x2000	/* Ku = 1, VSID = 0 */
	li	r10,0
3:	mtsrin	r8,r10
	addi	r8,r8,0x111	/* increment VSID */
	addis	r10,r10,0x1000	/* address of next segment */
	bdnz	3b
	blr

	.globl	disable_6xx_l1cache
disable_6xx_l1cache:
	/* Enable, invalidate and then disable the L1 icache/dcache. */
	li	r8,0
	ori	r8,r8,(HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI)
	mfspr	r11,SPRN_HID0
	or	r11,r11,r8
	andc	r10,r11,r8
	isync
	mtspr	SPRN_HID0,r8
	sync
	isync
	mtspr	SPRN_HID0,r10
	sync
	isync
	blr

/* udelay (on non-601 processors) needs to know the period of the
 * timebase in nanoseconds.  This used to be hardcoded to be 60ns
 * (period of 66MHz/4).  Now a variable is used that is initialized to
 * 60 for backward compatibility, but it can be overridden as necessary
 * with code something like this:
 *    extern unsigned long timebase_period_ns;
 *    timebase_period_ns = 1000000000 / bd->bi_tbfreq;
 */
	.data
	.globl timebase_period_ns
timebase_period_ns:
	.long	60

	.text
/*
 * Delay for a number of microseconds
 */
	.globl	udelay
udelay:
	mfspr	r4,SPRN_PVR
	srwi	r4,r4,16
	cmpwi	0,r4,1		/* 601 ? */
	bne	.udelay_not_601
00:	li	r0,86	/* Instructions / microsecond? */
	mtctr	r0
10:	addi	r0,r0,0 /* NOP */
	bdnz	10b
	subic.	r3,r3,1
	bne	00b
	blr

.udelay_not_601:
	mulli	r4,r3,1000	/* nanoseconds */
	/*  Change r4 to be the number of ticks using:	
	 *	(nanoseconds + (timebase_period_ns - 1 )) / timebase_period_ns
	 *  timebase_period_ns defaults to 60 (16.6MHz) */
	lis	r5,timebase_period_ns@ha
	lwz	r5,timebase_period_ns@l(r5)
	add	r4,r4,r5
	addi	r4,r4,-1
	divw	r4,r4,r5	/* BUS ticks */
1:	mftbu	r5
	mftb	r6
	mftbu	r7
	cmpw	0,r5,r7
	bne	1b		/* Get [synced] base time */
	addc	r9,r6,r4	/* Compute end time */
	addze	r8,r5
2:	mftbu	r5
	cmpw	0,r5,r8
	blt	2b
	bgt	3f
	mftb	r6
	cmpw	0,r6,r9
	blt	2b
3:	blr

	.section ".relocate_code","xa"
/*
 * Flush and enable instruction cache
 * First, flush the data cache in case it was enabled and may be
 * holding instructions for copy back.
 */
	.global flush_instruction_cache
flush_instruction_cache:
	mflr	r6
	bl	flush_data_cache

	/* Enable, invalidate and then disable the L1 icache/dcache. */
	li	r3,0
	ori	r3,r3,(HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI)
	mfspr	r4,SPRN_HID0
	or	r5,r4,r3
	isync
	mtspr	SPRN_HID0,r5
	sync
	isync
	ori	r5,r4,HID0_ICE	/* Enable cache */
	mtspr	SPRN_HID0,r5
	sync
	isync
	mtlr	r6
	blr

#define NUM_CACHE_LINES 128*8
#define cache_flush_buffer 0x1000

/*
 * Flush data cache
 * Do this by just reading lots of stuff into the cache.
 */
	.global flush_data_cache
flush_data_cache:
	lis	r3,cache_flush_buffer@h
	ori	r3,r3,cache_flush_buffer@l
	li	r4,NUM_CACHE_LINES
	mtctr	r4
00:	lwz	r4,0(r3)
	addi	r3,r3,32	/* (L1_CACHE_BYTES) Next line, please */
	bdnz	00b
10:	blr

	.previous

