/*
 * Copyright (c) 2008-2015 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "compiler_S.h"

/* #include "arch/cache.h" */
#define ICACHE                  2               //The old value is 1
#define DCACHE                  1               //The old value is 2
#define UCACHE                  (ICACHE|DCACHE)

#define __CACHE_ALIGN           __ALIGNED(CONFIG_ARCH_CACHE_LINE)

/* macros to align and unalign the stack on 8 byte boundary for ABI compliance */
.macro stack_align, tempreg
    /* make sure the stack is aligned */
    mov     \tempreg, sp
    tst     sp, #4
    subeq   sp, #4
    push    { \tempreg }

    /* tempreg holds the original stack */
.endm

.macro stack_restore, tempreg
    /* restore the potentially unaligned stack */
    pop     { \tempreg }
    mov     sp, \tempreg
.endm

/* save and disable the vfp unit */
.macro vfp_save, temp1
    /* save old fpexc */
    vmrs    \temp1, fpexc

    push    { \temp1 }

    /* hard disable the vfp unit */
    bic     \temp1, #(1<<30)
    vmsr    fpexc, \temp1
.endm

/* restore the vfp enable/disable state */
.macro vfp_restore, temp1
    /* restore fpexc */
    pop     { \temp1 }

    vmsr    fpexc, \temp1
.endm

/* Save callee trashed registers.
 * At exit r0 contains a pointer to the register frame.
 */
.macro save
    /* save spsr and r14 onto the svc stack */
    srsdb   #0x13!

    /* switch to svc mode, interrupts disabled */
    cpsid   i,#0x13

    /* save callee trashed regs and lr */
    push    { r0-r4, r12, lr }

    /* save user space sp/lr */
    sub     sp, #8
    stmia   sp, { r13, r14 }^

#if CONFIG_ARCH_USE_FPU
    /* save and disable the vfp unit */
    vfp_save    r0
#endif

    /* make sure the stack is 8 byte aligned */
    stack_align r0

    /* r0 now holds the pointer to the original iframe (before alignment) */
.endm

.macro save_offset, offset
    sub     lr, \offset
    save
.endm

.macro restore
    /* undo the stack alignment we did before */
    stack_restore r0

#if CONFIG_ARCH_USE_FPU
    /* restore the old state of the vfp unit */
    vfp_restore r0
#endif

    /* restore user space sp/lr */
    ldmia   sp, { r13, r14 }^
    add     sp, #8

    pop     { r0-r4, r12, lr }

    /* return to whence we came from */
    rfeia   sp!
.endm

/* Save all registers.
 * At exit r0 contains a pointer to the register frame.
 */
.macro saveall
    /* save spsr and r14 onto the svc stack */
    srsdb   #0x13!

    /* switch to svc mode, interrupts disabled */
    cpsid   i,#0x13

    /* save all regs */
    push    { r0-r12, lr }

    /* save user space sp/lr */
    sub     sp, #8
    stmia   sp, { r13, r14 }^

#if CONFIG_ARCH_USE_FPU
    /* save and disable the vfp unit */
    vfp_save    r0
#endif

    /* make sure the stack is 8 byte aligned */
    stack_align r0

    /* r0 now holds the pointer to the original iframe (before alignment) */
.endm

.macro saveall_offset, offset
    sub     lr, \offset
    saveall
.endm

.macro restoreall
    /* undo the stack alignment we did before */
    stack_restore r0

#if CONFIG_ARCH_USE_FPU
    /* restore the old state of the vfp unit */
    vfp_restore r0
#endif

    /* restore user space sp/lr */
    ldmia   sp, { r13, r14 }^
    add     sp, #8

    pop     { r0-r12, r14 }

    /* return to whence we came from */
    rfeia   sp!
.endm

FUNCTION(arm_save_mode_regs)
    mrs     r1, cpsr

    stmia   r0, { r13, r14 }^ /* usr */
    add     r0, #8

    cps     #0x11   /* fiq */
    str     r13, [r0], #4
    str     r14, [r0], #4

    cps     #0x12   /* irq */
    str     r13, [r0], #4
    str     r14, [r0], #4

    cps     #0x13   /* svc */
    str     r13, [r0], #4
    str     r14, [r0], #4

    cps     #0x17   /* abt */
    str     r13, [r0], #4
    str     r14, [r0], #4

    cps     #0x1b   /* und */
    str     r13, [r0], #4
    str     r14, [r0], #4

    cps     #0x1f   /* sys */
    str     r13, [r0], #4
    str     r14, [r0], #4

    msr     cpsr_c, r1

    bx      lr

FUNCTION(arm_undefined)
    save
    /* r0 now holds pointer to iframe */

    bl      arm_undefined_handler

    restore

FUNCTION(arm_prefetch_abort)
    saveall_offset #4
    /* r0 now holds pointer to iframe */

    bl      arm_prefetch_abort_handler

    restoreall

FUNCTION(arm_data_abort)
    saveall_offset #8
    /* r0 now holds pointer to iframe */

    bl      arm_data_abort_handler

    restoreall

FUNCTION(arm_reserved)
    b   .

