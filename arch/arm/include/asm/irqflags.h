/**
 *  @irqflag.h
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

#ifndef __ASM_ARM_IRQFLAGS_H
#define __ASM_ARM_IRQFLAGS_H

#ifdef __KERNEL__

#include <asm/ptrace.h>
#undef SWIENABLE
#define SWIENABLE "swi 0x190\n"

/*
 * CPU interrupt mask handling.
 */
#if __LINUX_ARM_ARCH__ >= 6

#ifdef CONFIG_CPU_OPT

#define raw_local_irq_save(x)					\
	({							\
	unsigned int virt_regs_ptr = CONFIG_VIRT_REGS_ADDR;	\
	unsigned int temp=0;	\
	x = 0;					\
	__asm__ __volatile__(					\
	"ldr %0, [%2, #0]\n"	\
	"mov %1, %3\n"	\
	"orr %1, %4, #0x80\n"	\
	"str %4, [%2, #0]\n"	\
	: "=r" (x), "=r" (temp)	\
	: "r" (virt_regs_ptr), "0" (x), "1" (temp)			\
	: "memory", "cc");				\
	})

//#define raw_local_irq_enable()  __asm__(SWIENABLE   "cpsie i	@ __sti" : : : "memory", "cc")
#define raw_local_irq_enable()  \
	({					\
	unsigned int virt_regs_ptr = CONFIG_VIRT_REGS_ADDR;	\
	 __asm__(				\
		"ldr r0, [%0, #0]\n"	\
		"bic r0, r0, #0x80\n"	\
		"str r0, [%0, #0]\n"	\
		:					\
		: "r" (virt_regs_ptr)					\
		: "r0", "memory", "cc");		\
	})

#define raw_local_irq_disable() 		\
	({							\
	unsigned int virt_regs_ptr = CONFIG_VIRT_REGS_ADDR;	\
	 __asm__(					\
		 "mov r1,%0\n"					\
		 "ldr r0,[r1]\n"				\
		 "orr r0,r0,#0x80 \n"			\
		 "str r0,[r1]\n"				\
		 :								\
		 : "r"(virt_regs_ptr)				\
		 : "r0","r1","memory", "cc");		\
	 })

#else /* CONFIG_CPU_OPT */
#define raw_local_irq_save(x)					\
	({							\
	__asm__ __volatile__(					\
	SWIENABLE                                               \
	"mrs	%0, cpsr		@ local_irq_save\n"	\
	SWIENABLE 						\
	"cpsid	i"						\
	: "=r" (x) : : "memory", "cc");				\
	})

#define raw_local_irq_enable()  __asm__(SWIENABLE   "cpsie i	@ __sti" : : : "memory", "cc")
#define raw_local_irq_disable() __asm__(SWIENABLE	"cpsid i	@ __cli" : : : "memory", "cc")
#endif /* CONFIG_CPU_OPT */

#define local_fiq_enable()  __asm__(SWIENABLE "cpsie f	@ __stf" : : : "memory", "cc")
#define local_fiq_disable() __asm__(SWIENABLE"cpsid f	@ __clf" : : : "memory", "cc")

#else

/*
 * Save the current interrupt enable state & disable IRQs
 */
#define raw_local_irq_save(x)					\
	({							\
		unsigned long temp;				\
		(void) (&temp == &x);				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ local_irq_save\n"	\
"	orr	%1, %0, #128\n"					\
"	msr	cpsr_c, %1"					\
	: "=r" (x), "=r" (temp)					\
	:							\
	: "memory", "cc");					\
	})
	
/*
 * Enable IRQs
 */
#define raw_local_irq_enable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ local_irq_enable\n"	\
"	bic	%0, %0, #128\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

/*
 * Disable IRQs
 */
#define raw_local_irq_disable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ local_irq_disable\n"	\
"	orr	%0, %0, #128\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

/*
 * Enable FIQs
 */
#define local_fiq_enable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ stf\n"		\
"	bic	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

/*
 * Disable FIQs
 */
#define local_fiq_disable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ clf\n"		\
"	orr	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

#endif

#ifdef CONFIG_CPU_OPT
/*#define raw_local_save_flags(x)					\
	({							\
	__asm__ __volatile__(					\
	SWIENABLE						\
	"mrs	%0, cpsr		@ local_save_flags"	\
	: "=r" (x) : : "memory", "cc");				\
	})*/

#define raw_local_save_flags(x)                 \
	({                          \
	unsigned int virt_regs_ptr = CONFIG_VIRT_REGS_ADDR;	\
	 __asm__ __volatile__(                   \
		"swi 0x214  @sync condition code\n" \
		"ldr %0, [%1, #0]\n"	\
		 : "=r" (x) \
		 : "r"(virt_regs_ptr)	\
		 : "memory", "cc");       \
	 })

#define raw_local_irq_restore(x) \
	({                                  \
	unsigned int virt_regs_ptr = CONFIG_VIRT_REGS_ADDR;	\
	 unsigned int tmp=0, tmp1=0;             \
	 __asm__ __volatile__(                   \
		 "ldr %0, [%3]\n"                    \
		 "bic %0, %4, #0xff\n"                                       \
		 "mov %1, %2\n"                                          \
		 "and %1, %5, #0xff\n"                                   \
		 "orr %0, %4, %1\n"                                      \
		 "str %4, [%3]\n"                                        \
		 : "=&r" (tmp), "=&r" (tmp1)             \
		 : "r"(x) ,"r"(virt_regs_ptr), "0"(tmp), "1"(tmp1)       \
		 : "memory", "cc");          \
	 }) 

#else

/*
 * Save the current interrupt enable state.
 */
#define raw_local_save_flags(x)					\
	({							\
	__asm__ __volatile__(					\
	SWIENABLE						\
	"mrs	%0, cpsr		@ local_save_flags"	\
	: "=r" (x) : : "memory", "cc");				\
	})

/*
 * restore saved IRQ & FIQ state
 */
#define raw_local_irq_restore(x)				\
	__asm__ __volatile__(					\
	SWIENABLE						\
	"msr	cpsr_c, %0		@ local_irq_restore\n"	\
	:							\
	: "r" (x)						\
	: "memory", "cc")
#endif //CONFIG_CPU_OPT

#define raw_irqs_disabled_flags(flags)	\
({					\
	(int)((flags) & PSR_I_BIT);	\
})

#endif
#endif
