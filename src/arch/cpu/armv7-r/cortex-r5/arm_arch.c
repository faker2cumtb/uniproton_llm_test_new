/*
 * arm_arch.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARMV7R arch function.
 *
 * Revision History:
 * -----------------
 */

#include "compiler.h"
#include "arch.h"
#include "arch/cache.h"
#include "arch/fpu.h"
#include "arch/pmu.h"
#include <prt_typedef.h>
#include <prt_cpu_external.h>

#ifdef CONFIG_ARCH_STACK_DYNAMIC
/* exception handler stacks */
#define __EXCEPT_STACK  __attribute((section(".except.stack")))// __SECTION(".except.stack")
uint8_t abt_stack[CONFIG_ARCH_ABT_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
uint8_t sys_stack[CONFIG_ARCH_SYS_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
uint8_t und_stack[CONFIG_ARCH_UND_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
uint8_t fiq_stack[CONFIG_ARCH_FIQ_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
uint8_t irq_stack[CONFIG_ARCH_IRQ_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
#if CONFIG_ARMV7R_TCMA_BASE == 0
uint8_t svc_stack[CONFIG_ARCH_SVC_STACK_SIZE] __CACHE_ALIGN __SECTION(".tcm.except.stack");
#else
uint8_t svc_stack[CONFIG_ARCH_SVC_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
#endif
#endif

uintptr_t g_sysStackHigh = (uintptr_t)(sys_stack + CONFIG_ARCH_SYS_STACK_SIZE);
uintptr_t g_svcStackHigh = (uintptr_t)(svc_stack + CONFIG_ARCH_SVC_STACK_SIZE);

/*
 * arch early init function.
 * call before kernel init.
 */
void arch_early_init(void)
{
    /* fpu config */
#ifdef CONFIG_ARCH_USE_FPU
    arm_fpu_enable();
#endif

}

/*
 * arch init function.
 * call after kernel init.
 */
void arch_init(void)
{
}

/*
 * arch idle function.
 */
void arch_idle(void)
{
    __ASM volatile("wfi");
}

/*
 * arch check exception stack.
 */
bool arch_check_exception_stack(void)
{
#ifdef CONFIG_ARCH_STACK_COLORATION
#ifdef CONFIG_ARCH_STACK_DYNAMIC
    if ((*(uint32_t *)abt_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)sys_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)und_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)fiq_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)irq_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)svc_stack != ARCH_STACK_COLOR)) {
        return false;
    }
#endif
#endif
    return true;
}
