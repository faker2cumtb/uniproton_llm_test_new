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

int task_del_hook()
{
    printf("start task_del_hook\n");
    
    U32 ret;
    TskHandle self, testTskHandle;
    TskPrior selfPrio;

    PRT_TaskSelf(&self);
    PRT_TaskGetPriority(self, &selfPrio);

    PRT_TaskDeleteHookAdd(hook_callback);
    hook_count =0;

    struct TskInitParam param = {0};
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = task_demo;
    param.name = "task_demo";
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
    if(hook_count != 0)
    {
        printf("hook_count is not 0, %d\n", hook_count);
        return PTS_FAIL;
    }

    ret = PRT_TaskDelete(testTskHandle);
    if (ret) {
        printf("Delete task fail, %u\n", ret);
        return -1;
    }

    if(hook_count != 1)
    {
        printf("hook_count is not 1, %d\n", hook_count);
        return PTS_FAIL;
    }

    ret = PRT_TaskDeleteHookDelete(hook_callback);
    if(ret)
    {
        printf("Delete hook fail, %u\n", ret);
        return -1;
    }

	return PTS_PASS;
}