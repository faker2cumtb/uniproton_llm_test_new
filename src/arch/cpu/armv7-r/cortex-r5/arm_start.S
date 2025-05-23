/*
 * arm_start.S
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM start function.
 *
 * Revision History:
 * -----------------
 */

#include "compiler_S.h"
#include "arm.h"

/* #include "arch/cache.h" */
#define ICACHE                  2               //The old value is 1
#define DCACHE                  1               //The old value is 2
#define UCACHE                  (ICACHE|DCACHE)

#define __CACHE_ALIGN           __ALIGNED(CONFIG_ARCH_CACHE_LINE)

/* #include "arch.h" */
#ifdef CONFIG_ARCH_STACK_COLORATION
#define ARCH_STACK_COLOR       0x5A5A5A5A
#endif
.globl _start

.section ".text.boot"

.arm
	.set SYS_MODE,	0x1f
	.set SVC_MODE,	0x13
	.set IRQ_MODE,	0x12
    .global  g_cfgPrimaryCore
    .global  g_gicCoreMap

    .type   g_cfgPrimaryCore, object
    .type   g_gicCoreMap, object

_start:
    ldr pc,   arm_reset_const
    ldr pc,   arm_undefined_const
    ldr pc,   arm_syscall_const
    ldr pc,   arm_prefetch_abort_const
    ldr pc,   arm_data_abort_const
    ldr pc,   arm_reserved_const
    ldr pc,   arm_irq_const
    ldr pc,   arm_fiq_const

arm_reset_const:                .word arm_reset
arm_undefined_const:            .word arm_undefined
arm_syscall_const:              .word osSysCallHandler
arm_prefetch_abort_const:       .word arm_undefined
arm_data_abort_const:           .word arm_undefined
arm_reserved_const:             .word arm_undefined
arm_irq_const:                  .word osIrqHandler
arm_fiq_const:                  .word arm_undefined

FUNCTION(arm_reset)
OsStackBssInit:
    /* do some early cpu setup */
    mov     r0, #(PSR_MODE_SVC | PSR_I_BIT | PSR_F_BIT)
    msr     cpsr_c, r0

    /* set little endian */
    SETEND  LE

    mrc     p15, 0, r12, c1, c0, 0
    bic     r12, r12, #(SCTLR_M | SCTLR_A | SCTLR_C)
    bic     r12, r12, #(SCTLR_SW | SCTLR_I | SCTLR_RR)
    bic     r12, r12, #(SCTLR_EE | SCTLR_TE)
    orr     r12, r12, #(SCTLR_Z)
    bic     r12, r12, #(SCTLR_V)
    mcr     p15, 0, r12, c1, c0, 0

    /* Configure peripheral ports */
    mrc     p15, 0, r0, c15, c0, 1
    tst     r0, #(0x1F << 2)
    beq     .Lno_normal_axi_pp
    /* Enable LLPP normal AXI interface */
    orr     r0, r0, #1
    mcr     p15, 0, r0, c15, c0, 1
.Lno_normal_axi_pp:
    mrc     p15, 0, r0, c15, c0, 2
    tst     r0, #(0x1F << 2)
    beq     .Lno_virtual_axi_pp
    /* Enable LLPP virtual AXI interface */
    orr     r0, r0, #1
    mcr     p15, 0, r0, c15, c0, 2
.Lno_virtual_axi_pp:
    mrc     p15, 0, r0, c15, c0, 3
    tst     r0, #(0x1F << 2)
    beq     .Lno_ahb_pp
    /* Enable AHB peripheral interface */
    orr     r0, r0, #1
    mcr     p15, 0, r0, c15, c0, 3
.Lno_ahb_pp:

    /* enable tcm before use stacks */
#ifdef CONFIG_ARMV7R_USE_TCMA
    ldr     r0, =CONFIG_ARMV7R_TCMA_BASE
    mov     r1, #0
    bl      tcma_enable_early       /* tcma 初始化 */
#endif

#ifdef CONFIG_ARMV7R_USE_TCMB
    ldr     r0, =CONFIG_ARMV7R_TCMB_BASE
    mov     r1, #0
    bl      tcmb_enable_early
#endif

#ifdef CONFIG_ARCH_STACK_DYNAMIC
.Lstack_setup:
    /* set up the stack for irq, fiq, abort, undefined, system/user, and lastly supervisor mode */
    mov     r12, #0

    /* Setup normal interrupt stack */
    cpsid   i, #MODE_IRQ
    ldr     r12, =irq_stack
    add     r12, #CONFIG_ARCH_IRQ_STACK_SIZE
    mov     sp, r12

    /* Setup fast interrupt stack */
    cpsid   i, #MODE_FIQ
    ldr     r12, =fiq_stack
    add     r12, #CONFIG_ARCH_FIQ_STACK_SIZE
    mov     sp, r12

    /* Setup data abort stack */
    cpsid   i, #MODE_ABT
    ldr     r12, =abt_stack
    add     r12, #CONFIG_ARCH_ABT_STACK_SIZE
    mov     sp, r12

    /* Setup undefined instruction stack */
    cpsid   i, #MODE_UND
    ldr     r12, =und_stack
    add     r12, #CONFIG_ARCH_UND_STACK_SIZE
    mov     sp, r12

    /* Setup system/user stack */
    cpsid   i, #MODE_SYS
    ldr     r12, =sys_stack
    add     r12, #CONFIG_ARCH_SYS_STACK_SIZE
    mov     sp, r12

    /* Setup supervisor stack */
    cpsid   i, #MODE_SVC
    ldr     r12, =svc_stack
    add     r12, #CONFIG_ARCH_SVC_STACK_SIZE
    mov     sp, r12

#endif

    /* enable cache, use stack enable after stack init */
#ifdef CONFIG_ARCH_EARLY_ENABLE_ICACHE
    ldr     r0, =ICACHE
    bl      arch_enable_cache
#endif

#ifdef CONFIG_ARCH_EARLY_ENABLE_DCACHE
    ldr     r0, =DCACHE
    bl      arch_enable_cache
#endif

#ifdef CONFIG_ARCH_STACK_DYNAMIC
    /* Init stack with stack magic.
     * Should be done after early copy to avoid to corrupt source
     * early copy data at their LMA.
     */
#ifdef CONFIG_ARCH_STACK_COLORATION
.Lstack_init:
    ldr     r4, =__except_stack_start
    ldr     r5, =__except_stack_end
    ldr     r6, =ARCH_STACK_COLOR
.Lstack_loop:
    cmp     r4, r5
    strlt   r6, [r4], #4
    blt     .Lstack_loop

#if CONFIG_ARMV7R_TCMA_BASE == 0
    ldr     r4, =__tcm_except_stack_start
    ldr     r5, =__tcm_except_stack_end
.Ltcm_stack_loop:
    cmp     r4, r5
    strlt   r6, [r4], #4
    blt     .Ltcm_stack_loop
#endif
#endif
#endif


.Lbss_init:
    /* clear out the bss */
    ldr     r4, =__bss_start
    ldr     r5, =__bss_end
    mov     r6, #0
.Lbss_loop:
    cmp     r4, r5
    strlt   r6, [r4], #4
    blt     .Lbss_loop

    bl      arch_early_init

OsPrimaryMain: /* OsPrimaryMain函数主核调用 */
    BL PRT_HardBootInit /* 用户调用的第一个函数钩子，用户在此处完成Seed随机数生成 */

#ifdef OS_GDB_STUB
    bl OsGdbStubInit
#endif

    bl       main
    b       .
