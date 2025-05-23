#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <securec.h>
#include <prt_config.h>
#include <prt_config_internal.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <prt_hwi.h>
#include <prt_sys.h>
#include <prt_tick.h>
#include <cpu_config.h>
#include <drv_common.h>
#include <prt_buildef.h>
#include <print.h>
#include <mica_service.h>

TskHandle g_mainTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

extern void apps_entry(void);

void osmain_entry(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
#ifdef OS_SUPPORT_CXX
    PRT_CppSystemInit();
#endif

#ifdef OS_GDB_STUB
    mica_service_init();
#endif

    apps_entry();
}

U32 OsMainTaskInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    // create task
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = osmain_entry;
#if defined(OS_SUPPORT_IGH_ETHERCAT) // ethercat 任务要比状态机任务优先级高
    param.taskPrio = 20;
#elif defined(DEMO_HRTIMER_TEST)
    param.taskPrio = 10; // 高精度定时器任务优先级较高
#else
    param.taskPrio = 30;
#endif
    param.name = "MainTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_mainTskHandle, &param);
    if (ret)
    {
        return ret;
    }

    ret = PRT_TaskResume(g_mainTskHandle);
    if (ret)
    {
        return ret;
    }

    return OS_OK;
}

U32 PRT_AppInit(void)
{
    U32 ret;

    /* Must init here */
    ret = drv_device_init();
    if (ret)
    {
        return ret;
    }
    
    ret = PRT_PrintfInit();
    if (ret)
    {
        return ret;
    }

    ret = OsMainTaskInit();
    if (ret)
    {
        return ret;
    }
    ret = TestClkStart();
    if (ret)
    {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
    U32 ret;

    ret = OsHwiInit();
    if (ret)
    {
        return ret;
    }

    return OS_OK;
}

void PRT_HardBootInit(void)
{
}

S32 main(void)
{
    return OsConfigStart();
}

extern void *__wrap_memset(void *dest, int set, size_t len)
{
    if (dest == NULL || len == 0)
    {
        return NULL;
    }

    char *ret = (char *)dest;
    for (int i = 0; i < len; ++i)
    {
        ret[i] = set;
    }
    return dest;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}
