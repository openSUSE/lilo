/*
 * arch/ppc/boot/simple/head.S
 *
 * Initial board bringup code for many different boards.
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

#include <ppc-reg.h>
#include <ppc_asm.h>

	.text

/*
 *      Begin at some arbitrary location in RAM or Flash
 *	  Initialize core registers
 *	  Configure memory controller (Not executing from RAM)
 *	Move the boot code to the link address (8M)
 *	  Setup C stack
 *	  Initialize UART
 *      Decompress the kernel to 0x0
 *      Jump to the kernel entry
 *
 */

	.globl	start_blr
start_blr:
	mr	r29,r3	/* On the MBX860, r3 is the board info pointer.
			 * On the RPXSUPER, r3 points to the NVRAM
			 * configuration keys.
			 * On PReP, r3 is the pointer to the residual data.
			 */

	mflr	r3	/* Save our actual starting address. */

	/* The following functions we call must not modify r3 or r4.....
	*/
	/* On PReP we must look at the OpenFirmware pointer and sanity
	 * test it.  On other platforms, we disable the MMU right now
	 * and other bits.
	 */
/*
 * Save the OF pointer to r25, but only if the entry point is in a sane
 * location; if not we store 0.  If there is no entry point, or it is
 * invalid, we establish the default MSR value immediately.  Otherwise,
 * we defer doing that, to allow OF functions to be called, until we
 * begin uncompressing the kernel.
 */
	lis	r8,0x0fff		/* r8 = 0x0fffffff */
	ori	r8,r8,0xffff

	subc	r8,r8,r5		/* r8 = (r5 <= r8) ? ~0 : 0 */
	subfe	r8,r8,r8
	nand	r8,r8,r8

	and.	r5,r5,r8		/* r5 will be cleared if (r5 > r8) */
	bne+	haveOF

	li	r8,MSR_IP|MSR_FP	/* Not OF: set MSR immediately */
  	mtmsr	r8
	isync
haveOF:
	mr	r25,r5
	bl	disable_6xx_l1cache


	mr	r4,r29	/* put the board info pointer where the relocate
			 * routine will find it
			 */

	/* Get the load address.
	*/
	subi	r3, r3, 4	/* Get the actual IP, not NIP */
	b	relocate

