#include "asm/ppc_asm.tmpl"
#include "asm/processor.h"

_GLOBAL(_start)
	lis	r9,_start@h
	ori	r9,r9,_start@l
	lis	r6,etext@ha
	addi	r6,r6,etext@l
1:	dcbf	r0,r9
	icbi	r0,r9
	addi	r9,r9,0x20
	cmplw	cr0,r9,r6
	blt	1b
	sync
	isync
	addi	r1,r1,-16
	rlwinm	r1,r1,0,0,27
	mr	r6,r1
	b	yaboot_start

#if 0
/*
 * Returns (address we're running at) - (address we were linked at)
 * for use before the text and data are mapped to KERNELBASE.
 */
_GLOBAL(reloc_offset)
	mflr	r0
	bl	1f
1:	mflr	r3
	lis	r4,1b@ha
	addi	r4,r4,1b@l
	subf	r3,r4,r3
	mtlr	r0
	blr
#endif
