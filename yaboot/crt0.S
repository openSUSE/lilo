#include "asm/ppc_asm.tmpl"
#include "asm/processor.h"

/*
 * Main entry point. should add code to clear BSS and more ...
 */
_GLOBAL(_start)
	b	yaboot_start

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
