/* $Id$ */
#include <ppc_asm.h>

	.text

	.globl	add_reloc_offset
add_reloc_offset:
	mflr	r0
	bl	1f
1:	mflr	r5
	lis	r4,1b@ha
	addi	r4,r4,1b@l
	subf	r5,r4,r5
	add	r3,r3,r5
	mtlr	r0
	blr

