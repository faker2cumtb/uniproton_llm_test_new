/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-07-24
 * Description: Hardware Initialization
 */
#include "prt_cpu_external.h"
#include "prt_sys_external.h"

#define MODE_SVC            0x13
#define PSR_T_BIT           (1 << 5)
#define PSR_F_BIT           (1 << 6)

#define OS_CPSR_REG_DEFAULT_VAL     (MODE_SVC|PSR_T_BIT|PSR_F_BIT)
#define OS_EXCRETURN_DEFAULT_VAL    0xFFFFFFFDU

/* Tick中断对应的硬件定时器ID */
// OS_SEC_DATA U32 g_tickTimerID = U32_INVALID;
// uintptr_t __os_sys_sp_end = (uintptr_t)(sys_stack + CONFIG_ARCH_SYS_STACK_SIZE);
// uintptr_t __os_sys_sp_start = (uintptr_t)(sys_stack);
// 系统栈配置

/*
 * 描述: 分配核的系统栈空间
 */
// INIT_SEC_L4_TEXT void InitSystemSp(uintptr_t sysStackHigh, uintptr_t svcStackHigh)
// {
//     return;
// }

/*
 * 描述: 获取系统栈的起始地址（低地址)
 */
// INIT_SEC_L4_TEXT uintptr_t OsGetSysStackStart(U32 core)
// {
//     (void)core;
//     return g_sysStackLow;
// }

/*
 * 描述: 获取系统栈的结束地址（高地址)
 */
// INIT_SEC_L4_TEXT uintptr_t OsGetSysStackEnd(U32 core)
// {
//     (void)core;
//     return g_sysStackHigh;
// }

// /*
//  * 描述: 获取系统栈的栈底（高地址)
//  */
// OS_SEC_L0_TEXT uintptr_t OsGetSysStackSP(U32 core)
// {
//     return OsGetSysStackEnd(core);
// }

INIT_SEC_L4_TEXT void *OsTskContextInit(U32 taskID, U32 stackSize, uintptr_t *topStack, uintptr_t funcTskEntry)
{
    (void)taskID;
    struct TskContext *stack = (struct TskContext *)((uintptr_t)topStack + stackSize);

    stack -= 1;
    stack->reg12 = 0x12121212;
    stack->reg11 = 0x11111111;
    stack->reg10 = 0x10101010;
    stack->reg9 = 0x09090909;
    stack->reg8 = 0x08080808;
    stack->reg7 = 0x07070707;
    stack->reg6 = 0x06060606;
    stack->reg5 = 0x05050505;
    stack->reg4 = 0x04040404;
    stack->reg3 = 0x03030303;
    stack->reg2 = 0x02020202;
    stack->reg1 = 0x01010101;
    stack->reg0 = 0x00000000;
    stack->lr = funcTskEntry;
    stack->spsr = OS_CPSR_REG_DEFAULT_VAL;
    stack->pc = funcTskEntry;

    return stack;
}

/*
 * 描述: 从指定地址获取任务上下文
 */
OS_SEC_L4_TEXT void OsTskContextGet(uintptr_t saveAddr, struct TskContext *context)
{
    *context = *((struct TskContext *)saveAddr);

    return;
}

/*
 * 描述: 手动触发异常（EL1）
 */
OS_SEC_L4_TEXT void OsAsmIll(void)
{
    OS_EMBED_ASM("svc  0");
}