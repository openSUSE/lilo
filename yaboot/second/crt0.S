#include "asm/ppc_asm.tmpl"

_GLOBAL(_yaboot_start)
	lis	r9,_start@h
	ori	r9,r9,_start@l
	lis	r6,_etext@ha
	addi	r6,r6,_etext@l
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

