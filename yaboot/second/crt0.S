#include "asm/ppc_asm.tmpl"
#include "asm/processor.h"

_GLOBAL(_start)
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
