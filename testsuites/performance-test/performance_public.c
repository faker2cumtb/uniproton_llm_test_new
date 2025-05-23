#include "performance_public.h"

#define _XOPEN_SOURCE 600
TskHandle stress_handle[100];
int stress_cnt;

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
#ifdef NOSTRESS_HRTIMER
    hrtimer_nostress_test,
#endif
#ifdef STRESS_HRTIMER
    hrtimer_stress_test,
#endif
#ifdef RELEASE_MUTEX_TIME
    release_mutex_test,
#endif
#ifdef INTERRUPT_GPIO
    interrupt_gpio_test,
#endif
#ifdef TASK_SWITCH
    task_switch_stress,
#endif
};

char run_test_name_1[][50] = {
#ifdef NOSTRESS_HRTIMER
    "hrtimer_nostress_test",
#endif
#ifdef STRESS_HRTIMER
    "hrtimer_stress_test",
#endif
#ifdef RELEASE_MUTEX_TIME
    "release_mutex_test",
#endif
#ifdef INTERRUPT_GPIO
    "interrupt_gpio_test",
#endif
#ifdef TASK_SWITCH
    "task_switch_stress",
#endif
};

U32 create_task(void *pfunc, TskPrior prio, char *name)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)pfunc;
    param.taskPrio = prio;

    param.name = name;
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&stress_handle[stress_cnt], &param);
    if (ret)
    {
        return ret;
    }

    ret = PRT_TaskResume(stress_handle[stress_cnt++]);
    if (ret)
    {
        return ret;
    }

    PRT_Printf("==========Create %s Success=========\n", name);
    return OS_OK;
}

void cpu_stress()
{
    sleep(1);
    while (true)
    {
        for (int i = 0; i < 1000; ++i)
        {
            for (int j = 1; j < 1000; ++j)
            {
                volatile double f = (double)(i + 0.2) * (double)(j + 0.5);
                (void)f;
            }
        }
        PRT_TaskDelay(1); // 让出CPU
    }
}

void mem_stress()
{
    sleep(1);
    const int buffer_size = 1 * 1024 * 1024; // 1MB大小的缓冲区
    char* buffer = (char*)malloc(buffer_size);
    uint64_t tmp = 0;

    // 先将缓冲区初始化
    for (int i = 0; i < buffer_size; i += sizeof(uint64_t)) {
        *(uint64_t *)(buffer + i) = tmp;
        tmp++;
    }

    while (true)
    {
        // 对1MB的内存块进行刷写操作
        for(int i = 0; i < buffer_size; i += sizeof(uint64_t)) {
            *(uint64_t *)(buffer + i) = tmp; // 向buffer0写入数据
            tmp++;
        }

        PRT_TaskDelay(1); // 让出CPU
    }
}

void io_stress()
{
    sleep(5);
    while (true)
    {
        int t = rand() % 10;
        for (int i = 0; i < t; ++i)
        {
            PRT_Printf("IO_stress IO_stress IO_stress IO_stress IO_stress IO_stress\n");
        }
        PRT_Printf("\n");
        PRT_TaskDelay(rand() % 200);
    }
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

    PRT_Printf("Start testing drivers....\n");

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
