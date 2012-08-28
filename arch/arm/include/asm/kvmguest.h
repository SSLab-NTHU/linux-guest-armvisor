/**
 *  @kvmguest.h
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

#ifndef __KVM_GUEST_H__
#define __KVM_GUEST_H__

#include <linux/types.h>
struct kvm_virt_regs {
	uint32_t cpsr;
	uint32_t c5_data;
	uint32_t c6_data;

	uint32_t svc_sp;
	uint32_t svc_lr;
	uint32_t svc_spsr;
	uint32_t usr_sp;
	uint32_t usr_lr;

	u32 vector_swi;
};
//extern unsigned int virt_regs_ptr;

#endif
