/*
 * Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H
#include <stdint.h>

#define RK_CLRSETBITS(clr, set)		((((clr) | (set)) << 16) | (set))
#define RK_SETBITS(set)			RK_CLRSETBITS(0, set)
#define RK_CLRBITS(clr)			RK_CLRSETBITS(clr, 0)

static inline void writel(uint32_t val, void *addr)
{
	*(volatile uint32_t *)addr = val;
}

static inline uint32_t readl(void *addr)
{
	return *(volatile uint32_t *)addr;
}

#define rk_clrsetreg(addr, clr, set)	\
				writel((((clr) | (set)) << 16) | (set), addr)
#define rk_clrreg(addr, clr)		writel((clr) << 16, addr)
#define rk_setreg(addr, set)		writel((set) << 16 | (set), addr)

#endif