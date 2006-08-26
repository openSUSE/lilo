/*
* Extended precision shifts.
*
* Updated to be valid for shift counts from 0 to 63 inclusive.
* -- Gabriel
*
* R3/R4 has 64 bit value
* R5    has shift count
* result in R3/R4
*
*  ashrdi3: arithmetic right shift (sign propagation)
*  lshrdi3: logical right shift
*  ashldi3: left shift
*/
#include "asm/ppc_asm.tmpl"

#if 0
	.globl __ashrdi3
__ashrdi3:
	subfic	r6,r5,32
	srw	r4,r4,r5	# LSW = count > 31 ? 0 : LSW >> count
	addi	r7,r5,32	# could be xori, or addi with -32
	slw	r6,r3,r6	# t1 = count > 31 ? 0 : MSW << (32-count)
	rlwinm	r8,r7,0,32	# t3 = (count < 32) ? 32 : 0
	sraw	r7,r3,r7	# t2 = MSW >> (count-32)
	or	r4,r4,r6	# LSW |= t1
	slw	r7,r7,r8	# t2 = (count < 32) ? 0 : t2
	sraw	r3,r3,r5	# MSW = MSW >> count
	or	r4,r4,r7	# LSW |= t2
	blr
#endif

	.globl __ashldi3
__ashldi3:
	subfic	r6,r5,32
	slw	r3,r3,r5	# MSW = count > 31 ? 0 : MSW << count
	addi	r7,r5,32	# could be xori, or addi with -32
	srw	r6,r4,r6	# t1 = count > 31 ? 0 : LSW >> (32-count)
	slw	r7,r4,r7	# t2 = count < 32 ? 0 : LSW << (count-32)
	or	r3,r3,r6	# MSW |= t1
	slw	r4,r4,r5	# LSW = LSW << count
	or	r3,r3,r7	# MSW |= t2
	blr

	.globl	__lshrdi3
__lshrdi3:
	subfic	r6,r5,32
	srw	r4,r4,r5	# LSW = count > 31 ? 0 : LSW >> count
	addi	r7,r5,32	# could be xori, or addi with -32
	slw	r6,r3,r6	# t1 = count > 31 ? 0 : MSW << (32-count)
	srw	r7,r3,r7	# t2 = count < 32 ? 0 : MSW >> (count-32)
	or	r4,r4,r6	# LSW |= t1
	srw	r3,r3,r5	# MSW = MSW >> count
	or	r4,r4,r7	# LSW |= t2
	blr

/* u64 __udivdi3(u64, u64) */
	.globl __udivdi3
__udivdi3:
	li	r7,0

/* u64 __udivmoddi4(u64, u64, u64 *) */
__udivmoddi4:
	stwu	r1,-32(r1)
	or.	r0,r5,r6
	stmw	r29,20(r1)
	mr	r31,r4
	mr	r30,r3
	mr	r29,r7
	li	r3,0
	li	r4,1
	bne-	2f
	trap
	li	r7,0
	li	r8,0
	b	7f
1:	mr	r5,r11
	mr	r6,r12
	mr	r3,r7
	mr	r4,r8
2:	cmpwi	cr7,r5,0
	rlwinm	r0,r6,1,31,31
	rlwinm	r9,r4,1,31,31
	rlwinm	r11,r5,1,0,30
	rlwinm	r7,r3,1,0,30
	rlwinm	r12,r6,1,0,30
	or	r11,r0,r11
	rlwinm	r8,r4,1,0,30
	or	r7,r9,r7
	bge+	cr7,1b
	li	r7,0
	li	r8,0
	b	6f
3:	bgt-	cr7,5f
	cmplw	cr7,r6,r31
	bne-	cr6,4f
	bgt-	cr7,5f
4:	subfc	r31,r6,r31
	subfe	r30,r5,r30
	addc	r8,r8,r4
	adde	r7,r7,r3
5:	rlwinm	r10,r6,31,1,31
	rlwinm	r9,r5,31,1,31
	mr	r3,r11
	mr	r4,r12
	or	r10,r0,r10
	mr	r5,r9
	mr	r6,r10
6:	or.	r9,r3,r4
	rlwinm	r0,r3,31,0,0
	rlwinm	r12,r4,31,1,31
	cmplw	cr7,r5,r30
	cmpw	cr6,r5,r30
	or	r12,r0,r12
	rlwinm	r11,r3,31,1,31
	rlwinm	r0,r5,31,0,0
	bne+	3b
	cmpwi	cr7,r29,0
	beq-	cr7,7f
	stw	r30,0(r29)
	stw	r31,4(r29)
7:	mr	r3,r7
	mr	r4,r8
	lmw	r29,20(r1)
	addi	r1,r1,32
	blr

/* u64 __umoddi3(u64, u64) */
	.globl __umoddi3
__umoddi3:
	mflr	r0
	stwu	r1,-32(r1)
	addi	r7,r1,8
	stw	r0,36(r1)
	bl	__udivmoddi4
	lwz	r0,36(r1)
	lwz	r3,8(r1)
	lwz	r4,12(r1)
	addi	r1,r1,32
	mtlr	r0
	blr

/* s64 __moddi3(s64, s64) */
	.globl __moddi3
__moddi3:
	mflr	r0
	stwu	r1,-32(r1)
	cmpwi	cr7,r3,0
	cmpwi	cr6,r5,0
	stw	r31,28(r1)
	stw	r0,36(r1)
	addi	r7,r1,8
	li	r31,0
	bge-	cr7,1f
	subfic	r4,r4,0
	subfze	r3,r3
	li	r31,1
1:	bge-	cr6,2f
	subfic	r6,r6,0
	subfze	r5,r5
	xori	r31,r31,1
2:	bl	__udivmoddi4
	cmpwi	cr7,r31,0
	beq-	cr7,3f
	lwz	r9,8(r1)
	lwz	r10,12(r1)
	subfic	r10,r10,0
	subfze	r9,r9
	stw	r9,8(r1)
	stw	r10,12(r1)
3:	lwz	r0,36(r1)
	lwz	r3,8(r1)
	lwz	r4,12(r1)
	lwz	r31,28(r1)
	addi	r1,r1,32
	mtlr	r0
	blr

/* s64 __divdi3(s64, s64) */
	.globl __divdi3
__divdi3:
	mflr	r0
	stwu	r1,-16(r1)
	cmpwi	cr7,r3,0
	li	r7,0
	cmpwi	cr6,r5,0
	stw	r31,12(r1)
	stw	r0,20(r1)
	li	r31,0
	bge-	cr7,1f
	subfic	r4,r4,0
	subfze	r3,r3
	li	r31,1
1:	bge-	cr6,2f
	subfic	r6,r6,0
	subfze	r5,r5
	xori	r31,r31,1
2:	bl	__udivmoddi4
	cmpwi	cr7,r31,0
	beq-	cr7,3f
	subfic	r4,r4,0
	subfze	r3,r3
3:	lwz	r0,20(r1)
	lwz	r31,12(r1)
	addi	r1,r1,16
	mtlr	r0
	blr

