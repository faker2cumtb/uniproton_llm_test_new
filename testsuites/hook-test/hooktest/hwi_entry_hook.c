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

int hwi_entry_hook()
{
    U32 ret;
    hook_count = 0;
    TskHandle self, testTskHandle;
    
    PRT_HwiAddEntryHook(hwi_hook_callback);

    struct TskInitParam param = {0};
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = timer_demo;
    param.name = "timer_demo";
    param.stackSize = 0x3000;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        printf("create task fail, %u\n", ret);
        return PTS_FAIL;
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        printf("resume task fail, %u\n", ret);
        return PTS_FAIL;
    }

    sleep(11);

    PRT_HwiDelEntryHook(hwi_hook_callback);

    if(hook_count == 20){
        printf("Test PASSED\n");
        return PTS_PASS;
    }else{
        printf("hook_count val is %d, not right\n", hook_count);
        printf("Test FAILED\n");
        return PTS_FAIL;
    } 
}