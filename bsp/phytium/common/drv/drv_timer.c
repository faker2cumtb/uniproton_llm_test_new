/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-02     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <fparameters.h>
#include <ftimer_tacho.h>
#include "drv_timer.h"

struct e2000q_timer
{
    char *name;
    u32 id;

    FTimerTachoCtrl ctrl;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    rt_hwtimer_t device;
    struct rt_hwtimer_info info;
};

#define _E2000Q_TIMER(_name, _id, _intr_num) \
    {                                        \
        .name = (_name),                     \
        .id = (_id),                         \
        .intr_num = (_intr_num),             \
        .intr_prio = 4,                      \
    }

static struct e2000q_timer timer_obj[] = {

#if defined(BSP_USING_TIMER0)
    _E2000Q_TIMER("timer0", 0, FTIMER_TACHO_IRQ_NUM(0)),
#endif

#if defined(BSP_USING_TIMER1)
    _E2000Q_TIMER("timer1", 1, FTIMER_TACHO_IRQ_NUM(1)),
#endif

#if defined(BSP_USING_TIMER2)
    _E2000Q_TIMER("timer2", 2, FTIMER_TACHO_IRQ_NUM(2)),
#endif

#if defined(BSP_USING_TIMER3)
    _E2000Q_TIMER("timer3", 3, FTIMER_TACHO_IRQ_NUM(3)),
#endif

#if defined(BSP_USING_TIMER4)
    _E2000Q_TIMER("timer4", 4, FTIMER_TACHO_IRQ_NUM(4)),
#endif

#if defined(BSP_USING_TIMER5)
    _E2000Q_TIMER("timer5", 5, FTIMER_TACHO_IRQ_NUM(5)),
#endif

#if defined(BSP_USING_TIMER6)
    _E2000Q_TIMER("timer6", 6, FTIMER_TACHO_IRQ_NUM(6)),
#endif

#if defined(BSP_USING_TIMER7)
    _E2000Q_TIMER("timer7", 7, FTIMER_TACHO_IRQ_NUM(7)),
#endif

#if defined(BSP_USING_TIMER8)
    _E2000Q_TIMER("timer8", 8, FTIMER_TACHO_IRQ_NUM(8)),
#endif

#if defined(BSP_USING_TIMER9)
    _E2000Q_TIMER("timer9", 9, FTIMER_TACHO_IRQ_NUM(9)),
#endif

#if defined(BSP_USING_TIMER10)
    _E2000Q_TIMER("timer10", 10, FTIMER_TACHO_IRQ_NUM(10)),
#endif

#if defined(BSP_USING_TIMER11)
    _E2000Q_TIMER("timer11", 11, FTIMER_TACHO_IRQ_NUM(11)),
#endif

#if defined(BSP_USING_TIMER12)
    _E2000Q_TIMER("timer12", 12, FTIMER_TACHO_IRQ_NUM(12)),
#endif

#if defined(BSP_USING_TIMER13)
    _E2000Q_TIMER("timer13", 13, FTIMER_TACHO_IRQ_NUM(13)),
#endif

#if defined(BSP_USING_TIMER14)
    _E2000Q_TIMER("timer14", 14, FTIMER_TACHO_IRQ_NUM(14)),
#endif

#if defined(BSP_USING_TIMER15)
    _E2000Q_TIMER("timer15", 15, FTIMER_TACHO_IRQ_NUM(15)),
#endif

#if defined(BSP_USING_TIMER16)
    _E2000Q_TIMER("timer16", 16, FTIMER_TACHO_IRQ_NUM(16)),
#endif

#if defined(BSP_USING_TIMER17)
    _E2000Q_TIMER("timer17", 17, FTIMER_TACHO_IRQ_NUM(17)),
#endif

#if defined(BSP_USING_TIMER18)
    _E2000Q_TIMER("timer18", 18, FTIMER_TACHO_IRQ_NUM(18)),
#endif

#if defined(BSP_USING_TIMER19)
    _E2000Q_TIMER("timer19", 19, FTIMER_TACHO_IRQ_NUM(19)),
#endif

#if defined(BSP_USING_TIMER20)
    _E2000Q_TIMER("timer20", 20, FTIMER_TACHO_IRQ_NUM(20)),
#endif

#if defined(BSP_USING_TIMER21)
    _E2000Q_TIMER("timer21", 21, FTIMER_TACHO_IRQ_NUM(21)),
#endif

#if defined(BSP_USING_TIMER22)
    _E2000Q_TIMER("timer22", 22, FTIMER_TACHO_IRQ_NUM(22)),
#endif

#if defined(BSP_USING_TIMER23)
    _E2000Q_TIMER("timer23", 23, FTIMER_TACHO_IRQ_NUM(23)),
#endif

#if defined(BSP_USING_TIMER24)
    _E2000Q_TIMER("timer24", 24, FTIMER_TACHO_IRQ_NUM(24)),
#endif

#if defined(BSP_USING_TIMER25)
    _E2000Q_TIMER("timer25", 25, FTIMER_TACHO_IRQ_NUM(25)),
#endif

#if defined(BSP_USING_TIMER26)
    _E2000Q_TIMER("timer26", 26, FTIMER_TACHO_IRQ_NUM(26)),
#endif

#if defined(BSP_USING_TIMER27)
    _E2000Q_TIMER("timer27", 27, FTIMER_TACHO_IRQ_NUM(27)),
#endif

#if defined(BSP_USING_TIMER28)
    _E2000Q_TIMER("timer28", 28, FTIMER_TACHO_IRQ_NUM(28)),
#endif

#if defined(BSP_USING_TIMER29)
    _E2000Q_TIMER("timer29", 29, FTIMER_TACHO_IRQ_NUM(29)),
#endif

#if defined(BSP_USING_TIMER30)
    _E2000Q_TIMER("timer30", 30, FTIMER_TACHO_IRQ_NUM(30)),
#endif

#if defined(BSP_USING_TIMER31)
    _E2000Q_TIMER("timer31", 31, FTIMER_TACHO_IRQ_NUM(31)),
#endif

#if defined(BSP_USING_TIMER32)
    _E2000Q_TIMER("timer32", 32, FTIMER_TACHO_IRQ_NUM(32)),
#endif

#if defined(BSP_USING_TIMER33)
    _E2000Q_TIMER("timer33", 33, FTIMER_TACHO_IRQ_NUM(33)),
#endif

#if defined(BSP_USING_TIMER34)
    _E2000Q_TIMER("timer34", 34, FTIMER_TACHO_IRQ_NUM(34)),
#endif

#if defined(BSP_USING_TIMER35)
    _E2000Q_TIMER("timer35", 35, FTIMER_TACHO_IRQ_NUM(35)),
#endif

#if defined(BSP_USING_TIMER36)
    _E2000Q_TIMER("timer36", 36, FTIMER_TACHO_IRQ_NUM(36)),
#endif

#if defined(BSP_USING_TIMER37)
    _E2000Q_TIMER("timer37", 37, FTIMER_TACHO_IRQ_NUM(37)),
#endif

};

static void e2000q_timer_intr_entry(uintptr_t arg)
{
    struct e2000q_timer *drv_timer = (struct e2000q_timer *)arg;

    FTimerTachoIntrHandler(0, &drv_timer->ctrl);
}

static void e2000q_timer_isr(void *arg)
{
    struct e2000q_timer *drv_timer = rt_container_of(arg, struct e2000q_timer, ctrl);

    rt_device_hwtimer_isr(&drv_timer->device);
}

void e2000q_timer_init(struct rt_hwtimer_device *timer, rt_uint32_t state)
{
    struct e2000q_timer *drv_timer = (struct e2000q_timer *)timer->parent.user_data;

    if (state == 1)
    {
        memset(&drv_timer->ctrl, 0, sizeof(drv_timer->ctrl));

        FTimerTachoConfig *cfg = &drv_timer->ctrl.config;
        FTimerGetDefConfig(drv_timer->id, cfg);

        cfg->work_mode = FTIMER_WORK_MODE_TIMER;
        cfg->timer_mode = FTIMER_RESTART;
        cfg->timer_bits = FTIMER_64_BITS;
        cfg->cmp_type = FTIMER_CYC_CMP;
        cfg->force_load = true;

        FTimerInit(&drv_timer->ctrl, cfg);
        FTimerSetStartVal(&drv_timer->ctrl, 0);
        FTimerSetPeriod64(&drv_timer->ctrl, UINT64_MAX);
    }
    else
    {
        FTimerStop(&drv_timer->ctrl);
        PRT_HwiDisable(drv_timer->intr_num);
    }
}

rt_err_t e2000q_timer_start(struct rt_hwtimer_device *timer, rt_uint32_t cnt, rt_hwtimer_mode_t mode)
{
    struct e2000q_timer *drv_timer = (struct e2000q_timer *)timer->parent.user_data;

    FTimerStop(&drv_timer->ctrl);
    PRT_HwiDisable(drv_timer->intr_num);

    FTimerTachoConfig *cfg = &drv_timer->ctrl.config;
    FTimerGetDefConfig(drv_timer->id, cfg);

    cfg->work_mode = FTIMER_WORK_MODE_TIMER;
    cfg->timer_bits = FTIMER_32_BITS;
    cfg->timer_mode = FTIMER_RESTART;
    cfg->cmp_type = FTIMER_CYC_CMP;
    cfg->force_load = true;
    FTimerRegisterEvtCallback(&drv_timer->ctrl, FTIMER_EVENT_CYC_CMP, e2000q_timer_isr);
    FTimerTachoSetIntr(&drv_timer->ctrl);

    FTimerInit(&drv_timer->ctrl, cfg);
    FTimerSetStartVal(&drv_timer->ctrl, 0);
    FTimerSetPeriod32(&drv_timer->ctrl, cnt);

    FTimerStart(&drv_timer->ctrl);

    if (PRT_HwiEnable(drv_timer->intr_num) != OS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

void e2000q_timer_stop(struct rt_hwtimer_device *timer)
{
    struct e2000q_timer *drv_timer = (struct e2000q_timer *)timer->parent.user_data;

    FTimerStop(&drv_timer->ctrl);
    PRT_HwiDisable(drv_timer->intr_num);
}

rt_uint32_t e2000q_timer_count_get(struct rt_hwtimer_device *timer)
{
    struct e2000q_timer *drv_timer = (struct e2000q_timer *)timer->parent.user_data;

    return FTimerGetCurCnt32(&drv_timer->ctrl);
}

rt_err_t e2000q_timer_control(struct rt_hwtimer_device *timer, rt_uint32_t cmd, void *args)
{
    return -RT_ENOSYS;
}

static const struct rt_hwtimer_ops e2000q_timer_ops = {
    .init = e2000q_timer_init,
    .start = e2000q_timer_start,
    .stop = e2000q_timer_stop,
    .count_get = e2000q_timer_count_get,
    .control = e2000q_timer_control,
};

rt_err_t e2000q_timer_config_init(struct e2000q_timer *drv_timer)
{
    drv_timer->info.maxfreq = FTIMER_CLK_FREQ_HZ;
    drv_timer->info.minfreq = FTIMER_CLK_FREQ_HZ;
    drv_timer->info.maxcnt = FTIMER_CLK_FREQ_HZ;
    drv_timer->info.cntmode = HWTIMER_CNTMODE_UP;

    drv_timer->device.info = &drv_timer->info;
    drv_timer->device.ops = &e2000q_timer_ops;

    return RT_EOK;
}

rt_err_t e2000q_timer_intr_init(struct e2000q_timer *drv_timer)
{
    U32 ret;

    ret = PRT_HwiSetAttr(drv_timer->intr_num, drv_timer->intr_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    ret = PRT_HwiCreate(drv_timer->intr_num, e2000q_timer_intr_entry, (uintptr_t)drv_timer);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    PRT_HwiSetRouter(drv_timer->intr_num);

    return RT_EOK;
}

int drv_timer_init()
{
    for (int i = 0; i < sizeof(timer_obj) / sizeof(timer_obj[0]); i++)
    {
        if (e2000q_timer_config_init(&timer_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_timer_intr_init(&timer_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rt_device_hwtimer_register(&timer_obj[i].device, timer_obj[i].name, &timer_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
