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
 * Description: 异常模块的对外头文件。
 */
#ifndef ARMV7_R_EXC_H
#define ARMV7_R_EXC_H

#include "prt_typedef.h"
#include "prt_sys.h"
#if defined(OS_OPTION_HAVE_FPU)
#include "os_cpu_armv7_r.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

struct ExcRegInfo {
    uintptr_t rregs[13];     // r0~r12
    uintptr_t lr;
    uintptr_t spsr;
    uintptr_t sp;
    uintptr_t pc;
    uintptr_t esr;
};

/*
 * CpuTick结构体类型。
 *
 * 用于记录64位的cycle计数值。
 */
struct SreCpuTick {
    U32 cntHi; /* cycle计数高32位 */
    U32 cntLo; /* cycle计数低32位 */
};

/*
 * 异常信息结构体
 */
struct ExcInfo {
    // OS版本号
    char osVer[OS_SYS_OS_VER_LEN];
    // 产品版本号
    char appVer[OS_SYS_APP_VER_LEN];
    // 异常原因
    U32 excCause;
    // 异常前的线程类型
    U32 threadType;
    // 异常前的线程ID, 该ID组成threadID = LTID
    U32 threadId;
    // 字节序
    U16 byteOrder;
    // CPU类型
    U16 cpuType;
    // CPU ID
    U32 coreId;
    // CPU Tick
    struct SreCpuTick cpuTick;
    // 异常嵌套计数
    U32 nestCnt;
    // 致命错误码，发生致命错误时有效
    U32 fatalErrNo;
    // 异常前栈指针
    uintptr_t sp;
    // 异常前栈底
    uintptr_t stackBottom;
    // 异常发生时的核内寄存器上下文信息
    struct ExcRegInfo regInfo;
};

/*
 * ARMV7_R异常具体类型:异常原因参见ESR寄存器。
 */
#define OS_EXCEPT_ESR           0

/*
 * ARMV7_R异常具体类型:其他核异常。
 */
#define OS_EXCEPT_OTHER_CORE    1

/*
 * ARMV7_R异常具体类型:致命错误异常。
 */
#define OS_EXCEPT_FATALERROR    2

/*
 * ARMV7_R异常具体类型:栈越界异常。
 */
#define OS_EXCEPT_STACKOVERFLOW 3

/*
 * ARMV7_R异常具体类型:非法指令异常。
 */
#define OS_EXCEPT_UNDEF_INSTR   4

/*
 * ARMV7_R异常具体类型:数据中止异常。
 */
#define OS_EXCEPT_DATA_ABORT    5

/*
 * ARMV7_R异常具体类型:快速中断异常。
 */
#define OS_EXCEPT_FIQ           6

/*
 * ARMV7_R异常具体类型:pc非对齐异常。
 */
#define OS_EXCEPT_PC_NOT_ALIGN  7

/*
 * ARMV7_R异常具体类型:sp非对齐异常。
 */
#define OS_EXCEPT_SP_NOT_ALIGN  8

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* ARMV7_R_EXC_H */
