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
 * Description: 硬中断。
 */
#include "prt_hwi_external.h"
#include "prt_task_external.h"
#include "prt_irq_external.h"
#include "prt_hwi_internal.h"
#include <stdio.h>

OS_SEC_DATA OsVoidFunc g_hwiSplLockHook = NULL;
OS_SEC_DATA OsVoidFunc g_hwiSplUnLockHook = NULL;
OS_SEC_DATA U32 ulPortYieldRequired = 0;
OS_SEC_DATA uintptr_t tmpSp;

/*
 * 描述: GIC模块初始化
 */
INIT_SEC_L4_TEXT void OsHwiGICInit(void)
{
    return;
}

/*
 * 描述: 获取硬中断优先级
 */
INIT_SEC_L4_TEXT U32 OsHwiPriorityGet(HwiHandle hwiNum)
{
    return OsGicGetPriority(hwiNum);
}

/*
 * 描述: 设置硬中断优先级
 */
INIT_SEC_L4_TEXT void OsHwiPrioritySet(HwiHandle hwiNum, HwiPrior hwiPrio)
{   
    OS_ERR_RECORD(OsGicSetPriority(hwiNum, hwiPrio));
}

/*
 * 描述: 禁止指定中断
 */
OS_SEC_L2_TEXT U32 PRT_HwiDisable(HwiHandle hwiNum)
{
    if (hwiNum > OS_HWI_MAX) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    OsGicDisableInt(hwiNum);

    return OS_OK;
}

/*
 * 描述: 使能指定中断
 */
OS_SEC_L2_TEXT U32 PRT_HwiEnable(HwiHandle hwiNum)
{
    if (hwiNum > OS_HWI_MAX) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    OsGicEnableInt(hwiNum);

    return OS_OK;
}

#if defined(OS_OPTION_SMP)
/*
 * 描述: 出发核间SGI中断，SMP调度频繁使用，放置OS_SEC_TEXT段
 */
OS_SEC_TEXT void OsHwiMcTrigger(enum OsHwiIpiType type, U32 coreMask, U32 hwiNum)
{
    /* 判断输入参数的合法性 */
    if (type >= OS_TYPE_TRIGGER_BUTT) {
        return OS_ERRNO_HWI_TRIGGER_TYPE_INVALID;
    }
    
    U32 coreList = coreMask;
    if (type == OS_TYPE_TRIGGER_TO_SELF) {
        coreList = (1U << OsGetCoreID());
    } else if (type == OS_TYPE_TRIGGER_TO_OTHER) {
        coreList = OS_ALLCORES_MASK;
        coreList &= ~(1U << OsGetCoreID());
        for(U32 coreId = 0; coreId < g_cfgPrimaryCore; coreId++) {
            coreList &= ~(1U << coreId);
        }
    }
    OsGicTrigIntToCores(hwiNum, coreList);
    return;
}

#else
/*
 * 描述: 触发核间SGI中断,放置OS_SEC_TEXT段
 */
OS_SEC_TEXT void OsHwiMcTrigger(U32 coreMask, U32 hwiNum)
{
    OsGicTrigIntToCores(hwiNum, coreMask);
    return;
}
#endif
/*
 * 描述: 上报中断号错误
 */
OS_SEC_TEXT void OsHwiReportHwiNumErr(void)
{
    OS_REPORT_ERROR(OS_ERRNO_HWI_HW_REPORT_HWINO_INVALID);
    return;
}

/*
 * 描述: 中断处理, 调用处外部已关中断
 */
OS_SEC_L0_TEXT void OsHwiDispatchHandle(void)
{
    U32 hwiNum;

    UNI_FLAG |= OS_FLG_HWI_ACTIVE;
    OS_INT_COUNT++;

    hwiNum = OsHwiNumGet();
    // printf("%d\n", hwiNum);
    if (OS_HWI_CLEAR_CHECK(hwiNum) || OS_HWI_NUM_CHECK(hwiNum)) {
        goto OS_HWI_CONTINUE;
    }

    OsHwiNestedIntEnable();
    OsHwiHookDispatcher(hwiNum);
    OsHwiNestedIntDisable();

OS_HWI_CONTINUE:
    // 清除中断位
    OsHwiClear(hwiNum);
    OS_INT_COUNT--;
    /* 不支持中断嵌套，但这里防止中断服务程序中错误打开了中断 */
    if (OS_INT_COUNT > 0) {
        printf("\n\n\n OS_INT_COUNT = %u \n\n\n",OS_INT_COUNT);
        return;
    }

    UNI_FLAG &= ~OS_FLG_HWI_ACTIVE;

    OsHwiDispatchTail();
}