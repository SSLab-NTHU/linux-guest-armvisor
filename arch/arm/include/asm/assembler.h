/*
 *  arch/arm/include/asm/assembler.h
 *
 *  Copyright (C) 1996-2000 Russell King
 *
 *  Copyright (C) 2009-2012 SSLab, NTHU (Support as a Guest OS for ARMvisor)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This file contains arm architecture specific defines
 *  for the different processors.
 *
 *  Do not include any C declarations in this file - it is included by
 *  assembler source.
 */
#ifndef __ASSEMBLY__
#error "Only include this from assembly code"
#endif

#include <asm/ptrace.h>
#define SWIENABLE swi 0x190
#ifdef CONFIG_CPU_OPT
#include <asm/kvm-asm.h>
#endif

/*
 * Endian independent macros for shifting bytes within registers.
 */
#ifndef __ARMEB__
#define pull            lsr
#define push            lsl
#define get_byte_0      lsl #0
#define get_byte_1	lsr #8
#define get_byte_2	lsr #16
#define get_byte_3	lsr #24
#define put_byte_0      lsl #0
#define put_byte_1	lsl #8
#define put_byte_2	lsl #16
#define put_byte_3	lsl #24
#else
#define pull            lsl
#define push            lsr
#define get_byte_0	lsr #24
#define get_byte_1	lsr #16
#define get_byte_2	lsr #8
#define get_byte_3      lsl #0
#define put_byte_0	lsl #24
#define put_byte_1	lsl #16
#define put_byte_2	lsl #8
#define put_byte_3      lsl #0
#endif

/*
 * Data preload for architectures that support it
 */
#if __LINUX_ARM_ARCH__ >= 5
#define PLD(code...)	code
#else
#define PLD(code...)
#endif

/*
 * This can be used to enable code to cacheline align the destination
 * pointer when bulk writing to memory.  Experiments on StrongARM and
 * XScale didn't show this a worthwhile thing to do when the cache is not
 * set to write-allocate (this would need further testing on XScale when WA
 * is used).
 *
 * On Feroceon there is much to gain however, regardless of cache mode.
 */
#ifdef CONFIG_CPU_FEROCEON
#define CALGN(code...) code
#else
#define CALGN(code...)
#endif

/*
 * Enable and disable interrupts
 */
#if __LINUX_ARM_ARCH__ >= 6
#ifdef CONFIG_CPU_OPT
	.macro disable_irq
	str r0, backup_regs
	str r1, backup_regs1
	ldr r0, virt_regs_ptr
	ldr r1, [r0, #VIRT_CPSR]
	orr r1, r1, #0x80
	str r1, [r0, #VIRT_CPSR]
	ldr r1, backup_regs1
	ldr r0, backup_regs
	.endm

	.macro enable_irq
	str r0, backup_regs
	str r1, backup_regs1
	ldr r0, virt_regs_ptr
	ldr r1, [r0, #VIRT_CPSR]
	bic r1, r1, #0x80
	str r1, [r0, #VIRT_CPSR]
	ldr r1, backup_regs1
	ldr r0, backup_regs
	.endm
#else
	.macro	disable_irq
	SWIENABLE
	cpsid	i
	.endm

	.macro	enable_irq
	SWIENABLE
	cpsie	i
	.endm
#endif //CONFIG_CPU_OPT

#else
	.macro	disable_irq
	msr	cpsr_c, #PSR_I_BIT | SVC_MODE
	.endm

	.macro	enable_irq
	msr	cpsr_c, #SVC_MODE
	.endm
#endif

/*
 * Save the current IRQ state and disable IRQs.  Note that this macro
 * assumes FIQs are enabled, and that the processor is in SVC mode.
 */
#ifdef CONFIG_CPU_OPT
	.macro  save_and_disable_irqs, oldcpsr
	str     r1, backup_regs_\oldcpsr
	str     r2, backup_regs2_\oldcpsr
	ldr     r1, virt_regs_ptr_\oldcpsr

	swi 0x214	@sync_cond_from_hw

	ldr     \oldcpsr,[r1, #0]
	mov     r2, \oldcpsr
	orr     r2, r2,#0x80
	str     r2, [r1, #0]
	ldr     r1, backup_regs_\oldcpsr
	ldr     r2, backup_regs2_\oldcpsr   
	b   run_\oldcpsr
	backup_regs_\oldcpsr: .word 0
	backup_regs2_\oldcpsr: .word 0
	virt_regs_ptr_\oldcpsr: .word CONFIG_VIRT_REGS_ADDR
	run_\oldcpsr:
	.endm
#else
	.macro	save_and_disable_irqs, oldcpsr
	SWIENABLE
	mrs	\oldcpsr, cpsr
	disable_irq
	.endm
#endif /* CONFIG_CPU_OPT */

/*
 * Restore interrupt state previously stored in a register.  We don't
 * guarantee that this will preserve the flags.
 */
#ifdef CONFIG_CPU_OPT
	.macro	restore_irqs, oldcpsr
	str     r1, backup_regs0_\oldcpsr
	str     r2, backup_regs1_\oldcpsr
	ldr     r1, virt_regs_\oldcpsr
	str     \oldcpsr, [r1, #0]
	ldr     r1, backup_regs0_\oldcpsr
	ldr     r2, backup_regs1_\oldcpsr
	b   restore_\oldcpsr
	backup_regs0_\oldcpsr: .word 0
	backup_regs1_\oldcpsr: .word 0
	virt_regs_\oldcpsr: .word CONFIG_VIRT_REGS_ADDR
	restore_\oldcpsr:
	.endm
#else
	.macro	restore_irqs, oldcpsr
	SWIENABLE
	msr	cpsr_c, \oldcpsr
	.endm
#endif /* CONFIG_CPU_OPT */

#define USER(x...)				\
9999:	x;					\
	.section __ex_table,"a";		\
	.align	3;				\
	.long	9999b,9001f;			\
	.previous

/*
 * SMP data memory barrier
 */
	.macro	smp_dmb
#ifdef CONFIG_SMP
#if __LINUX_ARM_ARCH__ >= 7
	dmb
#elif __LINUX_ARM_ARCH__ == 6
	mcr	p15, 0, r0, c7, c10, 5	@ dmb
#endif
#endif
	.endm
