/*
 * arch.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: arch interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_H
#define INCLUDE_ARCH_H

#include "compiler.h"
#include <stdbool.h>
#include <stdint.h>

typedef uintptr_t addr_t;
#ifdef CONFIG_ARCH_STACK_COLORATION
#define ARCH_STACK_COLOR       0x5A5A5A5A
#endif

#ifndef __ASSEMBLY__

__BEGIN_CDECLS

/*
 * arch early init before kernel start.
 */
void arch_early_init(void);

/*
 * arch init after kernel start.
 */
void arch_init(void);

/*
 * arch idle.
 */
void arch_idle(void);

/*
 * arch check exception stack.
 *
 * @return  check result.
 */
// bool OS_CRITICAL_CODE arch_check_exception_stack(void);
bool arch_check_exception_stack(void);

extern uint8_t sys_stack[];
extern uint8_t irq_stack[];

__END_CDECLS

#endif

#endif
