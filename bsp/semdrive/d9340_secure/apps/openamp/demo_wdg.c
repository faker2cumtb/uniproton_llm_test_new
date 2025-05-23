#include <stdio.h>
#include <string.h>
#include "demo.h"
#include "watchdog.h"
#include "irq_num.h"
#include "regs_base.h"
#include "print.h"
#include "prt_clk.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"

static int wdt_overflow_isr(uint32_t irq, void *arg);
static watchdog_device_t g_wdt;
static sdrv_watchdog_device_t g_sdrv_wdt = {

    .base  =  APB_WDT1_BASE,
    .irq   =  WDT1_OVFLOW_INTR_NUM,

    .isr   =  wdt_overflow_isr,
};

static int isr_prt = 0;
TskHandle g_testTskHandle12;

static int wdt_overflow_isr(uint32_t irq, void *arg)
{
    PRT_Printf("\nwatchdog overflow isr\n");
    isr_prt++;
    // PRT_TaskResume(g_testTskHandle12);
    return 0;
}
static U64 task01i = 0;
void Task0xxxz1()
{
    while(1)
    {
        task01i++;
        PRT_TaskSuspend(g_testTskHandle12);
    }
}

void watchdog_Demo(void)
{
    PRT_Printf("watchdog_Demo\n");
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)Task0xxxz1;
    param.taskPrio = 20;
    param.name = "TestTask1";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_testTskHandle12, &param);
    if (ret) {
        return ret;
    }

    int refresh_cnt = 1;
    int a = 10, b = 255, c = 9293, d = 12434, e = 129380;
    if (sdrv_watchdog_init(&g_wdt, &g_sdrv_wdt)) {
        PRT_Printf("sdrv_watchdog_init fail.\n");
        goto ret;
    }

    /* set timeout 2second */
    if (watchdog_set_timeout(&g_wdt, 1)) {
        PRT_Printf("watchdog_set_timeout fail.\n");
        goto ret;
    }

    if (watchdog_start(&g_wdt)) {
        PRT_Printf("watchdog_start fail.\n");
        goto ret;
    }
    
    while (refresh_cnt > 0) {
        PRT_ClkDelayMs(600);
        PRT_Printf("ping dog at time: %d\n", refresh_cnt);
        if(watchdog_ping(&g_wdt)) {
            PRT_Printf("watchdog_ping fail.\n");
            goto ret;
        }
        refresh_cnt--;
    }
    isr_prt = 0;

    PRT_Printf("Test watchdog timeout begin.zzzz\n");
    printf("a = %d, b = %d, c = %d, d = %d, e = %d\n", a, b, c, d, e);

    // 测试中断上下文恢复
    PRT_ClkDelayMs(960);
    for(int i = 0; i < 200; i++){
            printf("%d ",i);
    }
    PRT_Printf("\n");

    if(isr_prt != 1){
        PRT_Printf("Test watchdog fail.\n");
    }else{
        PRT_Printf("Test watchdog success.\n");
    }

    PRT_Printf("zzzz a = %d, b = %d, c = %d, d = %d, e = %d\n", a, b, c, d, e);

    // 长时间测试
    double floast = 1.234567891;
    double floast112 = 1.111111121;
    while(1)
    {
        PRT_ClkDelayMs(1000);
        // watchdog_ping(&g_wdt);
        PRT_Printf("task01i=%llu :%llu:%02llu:%02llu:  %.9f + %.9f = %.9f\n", task01i, PRT_TickGetCount()/3600000, (PRT_TickGetCount()%3600000)/60000, (PRT_TickGetCount()%60000)/1000, floast, floast112, floast + floast112);
    }
    watchdog_stop(&g_wdt);
ret:
    return;
}