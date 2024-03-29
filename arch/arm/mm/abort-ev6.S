/*
 * Copyright (c) 2009~2012  SSLab, NTHU (add patches for ARMvisor)
 *
 * Under GNU GPLv2 License
 */

#include <linux/linkage.h>
#include <asm/assembler.h>
#include "abort-macro.S"
/*
 * Function: v6_early_abort
 *
 * Params  : r2 = address of aborted instruction
 *         : r3 = saved SPSR
 *
 * Returns : r0 = address of abort
 *	   : r1 = FSR, bit 11 = write
 *	   : r2-r8 = corrupted
 *	   : r9 = preserved
 *	   : sp = pointer to registers
 *
 * Purpose : obtain information about current aborted instruction.
 * Note: we read user space.  This means we might cause a data
 * abort here if the I-TLB and D-TLB aren't seeing the same
 * picture.  Unfortunately, this does happen.  We live with it.
 */
	.align	5
#include <asm/kvm-asm.h>
ENTRY(v6_early_abort)
#ifdef CONFIG_CPU_32v6K
	clrex
#else
	sub	r1, sp, #4			@ Get unused stack location
	strex	r0, r1, [r1]			@ Clear the exclusive monitor
#endif
	virt_read_copr_data r1, VIRT_C5_DATA, "mrc	p15, 0, r1, c5, c0, 0"	@ get FSR
	virt_read_copr_data r0, VIRT_C6_DATA, "mrc	p15, 0, r0, c6, c0, 0"	@ get FAR
/*
 * Faulty SWP instruction on 1136 doesn't set bit 11 in DFSR (erratum 326103).
 * The test below covers all the write situations, including Java bytecodes
 */
	bic	r1, r1, #1 << 11		@ clear bit 11 of FSR
	tst	r3, #PSR_J_BIT			@ Java?
	movne	pc, lr
	do_thumb_abort
	ldreq	r3, [r2]			@ read aborted ARM instruction
#ifdef CONFIG_CPU_ENDIAN_BE8
	reveq	r3, r3
#endif
	do_ldrd_abort
	tst	r3, #1 << 20			@ L = 0 -> write
	orreq	r1, r1, #1 << 11		@ yes.
	mov	pc, lr


