#include "prt_sys.h"
#include "prt_tick.h"
#include "prt_config.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "cpu_config.h"
#include "securec.h"
#include "sdrv_tmr.h"
#include "regs_base.h"
#include "irq_num.h"
#include <stdio.h>

uint64_t g_timerFrequency;
#define PMU_TIMER_FREQUENCY g_timerFrequency
extern TskHandle g_testTskHandle12;
static sdrv_tmr_dev_t timer_dev = {
    .base = APB_TIMER3_BASE,
    .irq = TIMER3_CHN_INTR_NUM,
    .ovf_irq = TIMER3_OVF_INTR_NUM,
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 0,
    .cascase_mode = true,
    .cnt_per_us = 12,
    .ptp_base = 0,
    .id = SDRV_TMR_CHN_A,
};

uint64_t GetGenericTimerFreq(void)
{
    return (uint64_t)(timer_dev.clk_frq / (timer_dev.clk_div + 1));
}

uint64_t PRT_ClkGetCycleCount64(void)
{
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    uint64_t uptime_cycles = (uint64_t)sdrv_tmr_lld_get_glb_cnt(timer_dev.base);
    PRT_HwiRestore(intSave);
    return uptime_cycles;
}

static uint64_t prev_cycle = 0;
static uint64_t curr_cycle = 0;
static uint64_t currticks = 0;
extern TskHandle g_testTskHandle12;
void TimerIsr(uintptr_t para)
{
    sdrv_tmr_lld_get_int_status(timer_dev.base);
    PRT_TaskResume(g_testTskHandle12);
    PRT_TickISR();
    PRT_ISB();
}

void CoreTimerInit(void)
{
    uint32_t cyc_per_tick = timer_dev.clk_frq / (timer_dev.clk_div + 1) / OS_TICK_PER_SECOND;

    sdrv_tmr_lld_timer_start(timer_dev.base, timer_dev.id, cyc_per_tick);
    return;
}

uint32_t CoreTimerStart(void)
{
    g_timerFrequency = GetGenericTimerFreq();

    CoreTimerInit();
    
    return OS_OK;
}


uint32_t TestClkStart(void)
{
    uint32_t ret;


    sdrv_tmr_lld_init(timer_dev.base, timer_dev.clk_sel, timer_dev.clk_div);
    sdrv_tmr_lld_set_cascase_mode(timer_dev.base, timer_dev.cascase_mode);
    
    ret = PRT_HwiSetAttr(timer_dev.irq, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }
    
    ret = PRT_HwiCreate(timer_dev.irq, (HwiProcFunc)TimerIsr, 0);
    if (ret != OS_OK) {
        return ret;
    }

    PRT_HwiEnable(timer_dev.irq);

    ret = PRT_HwiSetAttr(timer_dev.ovf_irq, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }
    
    ret = PRT_HwiCreate(timer_dev.ovf_irq, (HwiProcFunc)TimerIsr, 0);
    if (ret != OS_OK) {
        return ret;
    }

    PRT_HwiEnable(timer_dev.ovf_irq);

    ret = CoreTimerStart();
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}


