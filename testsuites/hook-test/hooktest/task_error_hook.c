#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "prt_buildef.h"
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include <time.h>
#include <unistd.h>
#include "runHookTest.h"

static TskHandle testTskHandle;

void error_demo(void)
{
    int* ptr = NULL;
    printf("enter error_demo\n");
    *ptr = 0;
    printf("leave error_demo\n");
}

static U32 excep_hook(struct ExcInfo *excInfo)
{
    printf("enter exception hook\n");

    return 0;
}

int task_error_hook()
{
    printf("start task_error_hook\n");

    U32 ret;
    TskHandle self, testTskHandle;
    TskPrior selfPrio;

    PRT_TaskSelf(&self);
    PRT_TaskGetPriority(self, &selfPrio);

    PRT_ExcRegHook(excep_hook);

    struct TskInitParam param = {0};
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = error_demo;
    param.name = "error_demo";
    param.stackSize = 0x3000;
    param.taskPrio = selfPrio;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        printf("create task fail, %u\n", ret);
        return -1;
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        printf("resume task fail, %u\n", ret);
        return -1;
    }
    sched_yield();

    printf("can't exec here\n");
	return PTS_PASS;
}