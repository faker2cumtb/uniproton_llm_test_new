/*
 * prt_dispatch.S
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM irq handler.
 *
 * Revision History:    
 * -----------------
 */
#include <prt_buildef.h>
#include <prt_asm_arm_external.h>

.extern ulPortYieldRequired
.extern OsMainSchedule

/* macros to align and unalign the stack on 8 byte boundary for ABI compliance */
.macro STACK_ALIGN, reg
    MOV     \reg, sp
    TST     SP, #4
    SUBEQ   SP, #4
    PUSH    { \reg }
.endm

.macro STACK_RESTORE, reg
    POP     { \reg }
    MOV     sp, \reg
.endm

.macro EXC_SP_SET stackBottom, stackSize, reg0, reg1
    mrc    p15, 0, \reg0, c0, c0, 5
    and    \reg0, \reg0, #0 /* get cpu id */
    mov    \reg1, #\stackSize
    mul    \reg1, \reg1, \reg0             /* calculate current cpu stack offset */
    ldr    \reg0, =\stackBottom
    sub    \reg0, \reg0, \reg1             /* calculate current cpu stack bottom */
    mov    sp, \reg0                       /* set  sp */
.endm

#define FUNCTION(x) .global x; .type x,STT_FUNC; x:
#define DATA(x) .global x; .type x,STT_OBJECT; x:

.set SYS_MODE,	0x1F
.set SVC_MODE,	0x13
.set IRQ_MODE,	0x12
.set USR_MODE,	0x10

FUNCTION(PRT_HwiLock)
    MRS     R0, CPSR               // 将CPSR的当前值存入R0
    CPSID   IF                     // 禁用中断（将CPSR中的I位设为1）
    BX      LR                     // 返回

FUNCTION(PRT_GetCpsr)
    MRS     R0, CPSR               // 将CPSR的当前值存入R0
    BX      LR                     // 返回

FUNCTION(PRT_GetSpsr)
    MRS     R0, sPSR               // 将CPSR的当前值存入R0
    BX      LR                     // 返回

FUNCTION(PRT_HwiUnLock)
    MRS     R0, CPSR               // 将CPSR的当前值存入R0
    CPSIE   IF                     // 使能中断（将CPSR中的I位设为0）
    BX      LR                     // 返回

FUNCTION(PRT_HwiRestore)
    MSR     cpsr_c, r0
    BX      LR


FUNCTION(OsTaskTrap)
    LDR    r1, =g_runningTask /* OsTaskTrap是函数调用过来，r0 r1寄存器是caller save，此处能直接使用 */
    LDR    r0, [r1] /* r0 is the &g_pRunningTask->sp */
/*
 * 描述: Task调度处理函数。 SMP调度走这个接口，X0 is SP
 */
OsTskTrapSmp:
    MRS     r1, SPSR
    PUSH    {r1}
    PUSH    {lr}
    PUSH    {r0-r12, lr}

    // 存入SP指针到g_pRunningTask->sp
    mov    r1, sp
    str    r1, [r0]   // x0 is the &g_pRunningTask->sp

    ldr    r0, =g_sysStackHigh
    ldr    r0, [r0]

    mov    sp, r0
    B      OsMainSchedule
loop1:
    B      loop1

FUNCTION(OsTskContextLoad)
    /* clear the flag of ldrex */
    CLREX
    /* switch to new task's sp */
    LDR     SP, [R0]

FUNCTION(OsContextLoad)
    POP     {R0-R12, LR}

    POP     {R0}
    POP     {R1}
    MSR     CPSR, R1
    BX      R0

FUNCTION(OsGetSp)
    mov     R0, SP          @ 读取PSP
    BX      LR


FUNCTION(arm_undefined)
    bl      errorEcur
looperrun:
    B      looperrun


FUNCTION(osSysCallHandler)
    bl      errorEcur
looperr:
    B      looperr

FUNCTION(osIrqHandler)
    SUB     LR, LR, #4

    /* save spsr and lr(svc's pc) onto the svc stack */
    SRSDB   #0x13!

    /* disable irq, switch to svc mode */
    CPSID   i, #0x13

    /* push caller saved regs as trashed regs */
    PUSH    {R0-R3, R12, LR}

    /* 8 bytes stack align */
    STACK_ALIGN     R0

    /*
     * save fpu regs in case in case those been
     * altered in interrupt handlers.
    PUSH_FPU_REGS   R0 */

    PUSH    {R4}
    MOV     R4, SP

    ldr     r0, =g_svcStackHigh
    ldr     r0, [r0]
    mov     sp, r0

    BLX     OsHwiDispatchHandle

    MOV     SP, R4
    POP     {R4}

    // ulPortYieldRequired 不为0时，表示需要任务切换
    LDR		r1, =ulPortYieldRequired
	LDR		r0, [r1]
	CMP		r0, #0
    BLNE    OsTaskTrap

    /* restore fpu regs
    POP_FPU_REGS    R0 */

    STACK_RESTORE   R0

OsIrqContextRestore:
    POP     {R0-R3, R12, LR}
    RFEIA   SP!