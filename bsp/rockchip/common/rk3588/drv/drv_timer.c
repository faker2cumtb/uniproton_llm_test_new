/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-12     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <hal_base.h>
#include "drv_timer.h"

struct rk3588_timer
{
    char *name;
    struct TIMER_REG *reg;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    rt_hwtimer_t device;
    struct rt_hwtimer_info info;
};

#define _RK3588_TIMER(_name, _reg, _intr_num) \
    {                                         \
        .name = (_name),                      \
        .reg = (_reg),                        \
        .intr_num = (_intr_num),              \
        .intr_prio = 8,                       \
    }

static struct rk3588_timer timer_obj[] = {
#if defined(BSP_USING_TIMER0)
    _RK3588_TIMER("timer0", TIMER0, 321),
#endif

#if defined(BSP_USING_TIMER1)
    _RK3588_TIMER("timer1", TIMER1, 322),
#endif

#if defined(BSP_USING_TIMER2)
    _RK3588_TIMER("timer2", TIMER2, 323),
#endif

#if defined(BSP_USING_TIMER3)
    _RK3588_TIMER("timer3", TIMER3, 324),
#endif

#if defined(BSP_USING_TIMER4)
    _RK3588_TIMER("timer4", TIMER4, 325),
#endif

#if defined(BSP_USING_TIMER5)
    _RK3588_TIMER("timer5", TIMER5, 326),
#endif

#if defined(BSP_USING_TIMER6)
    _RK3588_TIMER("timer6", TIMER6, 327),
#endif

#if defined(BSP_USING_TIMER7)
    _RK3588_TIMER("timer7", TIMER7, 328),
#endif

#if defined(BSP_USING_TIMER8)
    _RK3588_TIMER("timer8", TIMER8, 329),
#endif

#if defined(BSP_USING_TIMER9)
    _RK3588_TIMER("timer9", TIMER9, 330),
#endif

#if defined(BSP_USING_TIMER10)
    _RK3588_TIMER("timer10", TIMER10, 331),
#endif

#if defined(BSP_USING_TIMER11)
    _RK3588_TIMER("timer11", TIMER11, 332),
#endif
};

static void rk3588_timer_isr(uintptr_t arg)
{
    struct rk3588_timer *drv_timer = (struct rk3588_timer *)arg;

    HAL_TIMER_ClrInt(drv_timer->reg);
    rt_device_hwtimer_isr(&drv_timer->device);
}

void rk3588_timer_init(struct rt_hwtimer_device *timer, rt_uint32_t state)
{
    struct rk3588_timer *drv_timer = (struct rk3588_timer *)timer->parent.user_data;

    if (state == 1)
    {
        /* RK3588只需要初始化，不需要启动 */
        HAL_TIMER_Stop(drv_timer->reg);
        HAL_TIMER_Init(drv_timer->reg, TIMER_USER_DEFINED);
        HAL_TIMER_SetCount(drv_timer->reg, UINT64_MAX);
    }
    else
    {
        HAL_TIMER_Stop(drv_timer->reg);
        HAL_TIMER_Stop_IT(drv_timer->reg);
        PRT_HwiDisable(drv_timer->intr_num);
    }
}

rt_err_t rk3588_timer_start(struct rt_hwtimer_device *timer, rt_uint32_t cnt, rt_hwtimer_mode_t mode)
{
    struct rk3588_timer *drv_timer = (struct rk3588_timer *)timer->parent.user_data;

    HAL_TIMER_Stop(drv_timer->reg);

    if (mode == HWTIMER_MODE_ONESHOT)
    {
        HAL_TIMER_Init(drv_timer->reg, TIMER_USER_DEFINED);
    }
    else
    {
        HAL_TIMER_Init(drv_timer->reg, TIMER_FREE_RUNNING);
    }

    HAL_TIMER_SetCount(drv_timer->reg, cnt);
    HAL_TIMER_Start_IT(drv_timer->reg);
    if (PRT_HwiEnable(drv_timer->intr_num) != OS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

void rk3588_timer_stop(struct rt_hwtimer_device *timer)
{
    struct rk3588_timer *drv_timer = (struct rk3588_timer *)timer->parent.user_data;

    HAL_TIMER_Stop(drv_timer->reg);
    HAL_TIMER_Stop_IT(drv_timer->reg);
    PRT_HwiDisable(drv_timer->intr_num);
}

rt_uint32_t rk3588_timer_count_get(struct rt_hwtimer_device *timer)
{
    struct rk3588_timer *drv_timer = (struct rk3588_timer *)timer->parent.user_data;

    return HAL_TIMER_GetCount(drv_timer->reg) & 0xffffffff;
}

rt_err_t rk3588_timer_control(struct rt_hwtimer_device *timer, rt_uint32_t cmd, void *args)
{
    return -RT_ENOSYS;
}

static const struct rt_hwtimer_ops rk3588_timer_ops = {
    .init = rk3588_timer_init,
    .start = rk3588_timer_start,
    .stop = rk3588_timer_stop,
    .count_get = rk3588_timer_count_get,
    .control = rk3588_timer_control,
};

rt_err_t rk3588_timer_config_init(struct rk3588_timer *drv_timer)
{
    rt_uint32_t osc_freq = 24000000;

    drv_timer->info.maxfreq = osc_freq;
    drv_timer->info.minfreq = osc_freq;
    drv_timer->info.maxcnt = osc_freq;

    if ((drv_timer->reg != TIMER5) && (drv_timer->reg != TIMER11))
    {
        drv_timer->info.cntmode = HWTIMER_CNTMODE_DW;
    }
    else
    {
        drv_timer->info.cntmode = HWTIMER_CNTMODE_UP;
    }

    drv_timer->device.ops = &rk3588_timer_ops;
    drv_timer->device.info = &drv_timer->info;

    return RT_EOK;
}

rt_err_t rk3588_timer_intr_init(struct rk3588_timer *drv_timer)
{
    U32 ret;

    ret = PRT_HwiSetAttr(drv_timer->intr_num, drv_timer->intr_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    ret = PRT_HwiCreate(drv_timer->intr_num, rk3588_timer_isr, (uintptr_t)drv_timer);
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
        if (rk3588_timer_config_init(&timer_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_timer_intr_init(&timer_obj[i]) != RT_EOK)
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
