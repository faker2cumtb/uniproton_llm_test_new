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

int task_create_hook()
{
    printf("start task_create_hook\n");

    hook_count = 0;
    
    U32 ret;
    TskHandle self, testTskHandle;
    TskPrior selfPrio;
    PRT_TaskSelf(&self);
    PRT_TaskGetPriority(self, &selfPrio);

    PRT_TaskCreateHookAdd(hook_callback);

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

    if(hook_count != 1){
        printf("create thread hook not called. Test FAILED.\n");
        return PTS_FAIL;
    }
    hook_count = 0;

    // 获取当前时间的时间戳
    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        printf("resume task fail, %u\n", ret);
        return -1;
    }

    ret = PRT_TaskDelete(testTskHandle);
    if (ret) {
        printf("Delete task fail, %u\n", ret);
        return -1;
    }

    ret = PRT_TaskCreateHookDelete(hook_callback);
    if(ret)
    {
         printf("PRT_TaskCreateHookDelete call failed, %u\n", ret);
         return PTS_FAIL;
    }

    return PTS_PASS;
}