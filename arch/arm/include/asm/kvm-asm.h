/**
 *  @kvm-asm.h
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *  Copyright (c) 2009~2012  SSLab, NTHU
 *
 */

#ifndef __KVM_ASM_H__
#define __KVM_ASM_H__

#include <asm/asm-offsets.h>
#undef SWIENABLE
#define SWIENABLE swi 0x190

virt_regs_ptr:
	.word CONFIG_VIRT_REGS_ADDR
backup_regs:
	.word 0
backup_regs1:
	.word 0

#ifdef CONFIG_CPU_OPT
	.macro virt_write_regs, Rm, offset, inst
	str sp, backup_regs
	ldr sp, virt_regs_ptr
	str \Rm, [sp, #\offset]
	ldr sp, backup_regs
	.endm

	.macro virt_read_regs, Rm, offset, inst
	str sp, backup_regs
	.ifc \offset, VIRT_CPSR
	swi 0x214	@sync_cond_from_hw
	.endif
	ldr sp, virt_regs_ptr
	ldr \Rm, [sp, #\offset]
	ldr sp, backup_regs
	.endm

	.macro virt_cps, inst
	str r0, backup_regs
	str r1, backup_regs1
	ldr r0, virt_regs_ptr
	ldr r1, [r0, #VIRT_CPSR]
	orr r1, r1, #0xc0
	str r1, [r0, #VIRT_CPSR]
	ldr r1, backup_regs1
	ldr r0, backup_regs
	.endm

	.macro virt_load_usr_regs, sp, Rf, Rt, inst
	.if \Rf != sp
	sub \sp, \sp, #8
	ldmdb \sp, {\Rf - r12}
	add \sp, \sp, #8
	.endif
	str r0, backup_regs
	str r1, backup_regs1
	ldr r1, virt_regs_ptr
	ldr r0, [\sp, #-8]
	str r0, [r1, #VIRT_USR_SP]
	ldr r0, [\sp, #-4]
	str r0, [r1, #VIRT_USR_LR]
	ldr r0, backup_regs
	ldr r1, backup_regs1
	.endm

	.macro virt_store_usr_regs, sp, inst
	str r5, backup_regs
	str r1, backup_regs1
	ldr r5, virt_regs_ptr
	ldr r1, [r5, #VIRT_USR_SP]
	str r1, [\sp, #-8]
	ldr r1, [r5, #VIRT_USR_LR]
	str r1, [\sp, #-4]
	ldr r5, backup_regs
	ldr r1, backup_regs1
	.endm

	.macro virt_svc_movs, inst
	str r0, backup_regs
	str r1, backup_regs1
	ldr r0, virt_regs_ptr

	ldr r1, [r0, #VIRT_SVC_SPSR]	@ get svc_spsr
	str r1, [r0, #VIRT_CPSR]           @ save to guest cpsr
 	and r1, r1, #0x1f
 	cmp r1, #0x10
	streq sp, [r0, #VIRT_SVC_SP]      @ save svc_sp, svc_lr into banked
	streq lr, [r0, #VIRT_SVC_LR]
 	ldreq sp, [r0, #VIRT_USR_SP]         @ read usr_sp, usr_lr
 	ldreq lr, [r0, #VIRT_USR_LR]
	ldr r0, backup_regs
	ldr r1, backup_regs1
	swieq 0x254
	mov pc, lr
	.endm

	.macro virt_read_copr_data, Rm, offset, inst
	str sp, backup_regs
	ldr sp, virt_regs_ptr
	ldr \Rm, [sp, #\offset]
	ldr sp, backup_regs
	.endm

#else
	.macro virt_write_regs, Rm, offset, inst
	SWIENABLE
	\inst
	.endm

	.macro virt_read_regs, Rm, offset, inst
	SWIENABLE
	\inst
	.endm

	.macro virt_cps, inst
	SWIENABLE
	\inst
	.endm

	.macro virt_load_usr_regs, sp, Rf, Rt, inst
	SWIENABLE
	\inst
	.endm

	.macro virt_store_usr_regs, sp, inst
	SWIENABLE
	\inst
	.endm

	.macro virt_svc_movs, inst
	SWIENABLE
	\inst
	.endm

	.macro virt_read_copr_data, Rm, offset, inst
	\inst
	.endm

#endif  /* CONFIG_CPU_OPT */
	.macro virt_exec, inst
	SWIENABLE
	\inst
	.endm

	.macro virt_swi, parm, type
	str r8, backup_regs
	str r9, backup_regs1
	ldr r8, virt_regs_ptr
	mov r9, #\type 
	str r9, [r8, #VIRT_SWI_TYPE]
	.ifc \type, 11
	ldr r9, [\parm]
	.endif
	str \parm, [r8, #VIRT_SWI_PARM]
	ldr r8, backup_regs
	ldr r9, backup_regs1
	swi 0x250
	.endm
#endif  /* __KVM_ASM_H__ */
