#include <prt_config_internal.h>
#include <prt_cpu_external.h>
#include <cpu_config.h>
#include <print.h>
#include <demo.h>

TskHandle testTskHandle[2];
U32 task_status;

void SlaveTaskEntry()
{
    PRT_Printf("Slave 1 running\n");
    U32 temp = 0;
    while (1)
    {
        PRT_Printf("Slave1 count %d\n", temp);
        PRT_TaskDelay(1000);
        ++temp;
        if(temp % 5 == 0){
            PRT_Printf("Trying to wake up slave2\n");
            task_status = PRT_TaskResume(testTskHandle[1]);
            if(task_status != OS_OK){
                PRT_Printf("can not resume task 1\n");
            }
        }
    }
}

void SlaveTaskEntry2()
{
    while (1)
    {
        PRT_Printf("Task 2 has been waked up\n");
        PRT_TaskSuspend(testTskHandle[1]);
    }
}

U32 SlaveTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    // task 1
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)SlaveTaskEntry;
    param.taskPrio = 25;
    param.name = "SlaveTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle[0], &param);
    if (ret) {
        return ret;
    }

    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)SlaveTaskEntry2;
    param.taskPrio = 20;
    param.name = "Test2Task";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle[1], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[0]);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

void smp_demo(){
    for(int cnt = 0; cnt < 50; cnt++)
    {
        PRT_Printf("Master = %d\n", cnt);
        PRT_TaskDelay(1000);
    }
}
