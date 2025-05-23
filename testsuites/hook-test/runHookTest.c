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

#define _XOPEN_SOURCE 600
int hook_count = 0;

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    idle_before_hook_test,
    hwi_entry_hook,
    hwi_exit_hook,
    task_create_hook,
    task_switch_hook,
    task_del_hook,
    //task_error_hook,                    //目前不支持异常返回,系统退出，不能自动化
    idle_hook_test,
};


char run_test_name_1[][50] = {
    "idle_before_hook_test",
    "hwi_entry_hook",
    "hwi_exit_hook",
    "task_create_hook",
    "task_switch_hook",
    "task_del_hook",
    //"task_error_hook",
    "idle_hook_test",
};

U32 hook_callback(TskHandle taskPid)
{
    print_time();
    
    hook_count++;
}

U32 hwi_hook_callback(U32 hwiNum)
{
    if(hwiNum == 226){
        print_time();
        hook_count++;
    }
}

void task_demo()
{
    printf("task_demo.\n");
    while (1)
    {
        sleep(1);
    }
}

void print_time()
{
    struct timespec ts;
    if(clock_gettime(CLOCK_REALTIME, &ts) == -1){
        printf("get time error");
        return 1;
    }

    printf("Current time: %ld seconds, %ld nanoseconds\n", ts.tv_sec, ts.tv_nsec);
}

long sysconf(int name)
{
    switch(name) {
        case _SC_CPUTIME:
        case _SC_THREAD_CPUTIME:
        case _SC_MONOTONIC_CLOCK:
            return 1;
        case _SC_SEM_NSEMS_MAX:
            return OS_SEM_COUNT_MAX;
        default:
            return 0;
    }
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    int runCount = 0;
    int failCount = 0;
    int i;
    int ret = 0;
    test_run_main *run;

    PRT_Printf("Start testing hook....\n");

    for (i = 0; i < sizeof(run_test_arry_1)/sizeof(test_run_main *); i++) {
        run = run_test_arry_1[i];
        PRT_Printf("Runing %s test...\n", run_test_name_1[i]);
        ret = run();
        if (ret != 0) {
            failCount++;
            PRT_Printf("Run %s test fail\n", run_test_name_1[i]);
        }
    }
    runCount += i;

    PRT_Printf("Run total testcase %d, failed %d\n", runCount, failCount);
}