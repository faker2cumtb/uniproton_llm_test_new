/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-22
 * Description: cpu架构相关的外部头文件
 */
#ifndef OS_CPU_ARMV7_R_H
#define OS_CPU_ARMV7_R_H

#include "prt_buildef.h"
#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * 任务上下文的结构体定义。
 */
struct TskContext {
    uintptr_t reg12;
    uintptr_t reg11;
    uintptr_t reg10;
    uintptr_t reg9;
    uintptr_t reg8;
    uintptr_t reg7;
    uintptr_t reg6;
    uintptr_t reg5;
    uintptr_t reg4;
    uintptr_t reg3;
    uintptr_t reg2;
    uintptr_t reg1;
    uintptr_t reg0;
    uintptr_t lr;
    uintptr_t pc;
    uintptr_t spsr;
};

/*
 *  描述: 读取当前核号
 */
OS_SEC_ALW_INLINE INLINE U32 OsGetCoreID(void)
{
    return 0;//单核运行，不需要获取核号
}

/*
 * 获取当前核ID
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_GetCoreID(void)
{
    return OsGetCoreID();
}

#define PRT_DSB() OS_EMBED_ASM("DSB sy" : : : "memory")
#define PRT_DMB() OS_EMBED_ASM("DMB sy" : : : "memory")
#define PRT_ISB() OS_EMBED_ASM("ISB" : : : "memory")

#define PRT_MemWait PRT_DSB

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* OS_CPU_ARMV7_R_H */
