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

static TskHandle self, testTskHandle;

static U32 tskSwitchHook(TskHandle lastTaskPid, TskHandle nextTaskPid)
{   
    print_time();
    
    if(lastTaskPid == self) {
        hook_count++;
    }
}

static void task_switch_demo()
{
    while (1)
    {
        //printf("====%d==\n", __LINE__);
        sched_yield();
    }
}

int task_switch_hook()
{
    printf("start task_switch_hook\n");

    U32 ret;
    
    TskPrior selfPrio;

    PRT_TaskSelf(&self);
    PRT_TaskGetPriority(self, &selfPrio);

    PRT_TaskAddSwitchHook(tskSwitchHook);
    hook_count = 0;

    struct TskInitParam param = {0};
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = task_switch_demo;
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
    if(hook_count != 1)
    {
        printf("hook_count != 1\n");
        return PTS_FAIL;
    }

    sched_yield();
    if(hook_count != 2)
    {
        printf("hook_count != 2\n");
        return PTS_FAIL;
    }

    hook_count = 0;
    ret = PRT_TaskDelSwitchHook(tskSwitchHook);
    if(ret != 0)
    {
        printf("del switch hook fail, %u\n", ret);
        return -1;
    }

    sched_yield();
    if(hook_count != 0)
    {
        printf("hook_count != 0\n");
        return PTS_FAIL;
    }

    ret = PRT_TaskDelete(testTskHandle);
    if (ret) {
        printf("Delete task fail, %u\n", ret);
        return -1;
    }

	return PTS_PASS;
}