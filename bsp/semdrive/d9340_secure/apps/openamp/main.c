#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "print.h"
#include "demo.h"

TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

void errorEcur(void){
    PRT_Printf("Error\n");
}


void TestTaskEntry()
{
    PRT_Printf("Hello, UniProton!\n");
    PRT_Printf("temp  SemDrive D9 @ %s %s\n", __DATE__, __TIME__);

    #if 0//defined(TESTSUITE_CASE)
    PRT_Printf("TESTSUITE_CASE\n");
    Init(0, 0, 0, 0);
    #else
    watchdog_Demo();
    #endif
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTaskEntry;
    param.taskPrio = 25;
    param.name = "TestTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_testTskHandle, &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle);
    if (ret) {
        return ret;
    }
    return OS_OK;
}

U32 PRT_AppInit(void)
{
    U32 ret;
    ret = OsTestInit();
    if (ret) {
        return ret;
    }

    ret = TestClkStart();
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
    U32 ret;

    ret = OsHwiInit();
    if (ret) {
        return ret;
    }
    return OS_OK;
}

void PRT_HardBootInit(void)
{
}

S32 main(void)
{
    OsConfigStart();
}

extern void *__wrap_memset(void *dest, int set, U32 len)
{
    if (dest == NULL || len == 0) {
        return NULL;
    }

    char *ret = (char *)dest;
    for (int i = 0; i < len; ++i) {
        ret[i] = set;
    }
    return dest;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}

