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
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <demo.h>

static rt_device_t timer_dev = RT_NULL;

static rt_err_t timeout_callback(rt_device_t dev, rt_size_t size)
{
    rt_hwtimerval_t tv;

    rt_device_read(timer_dev, 0, &tv, sizeof(tv));

    PRT_Printf("timeout = %d.%06d s\n", tv.sec, tv.usec);

    return 0;
}

void timer_demo()
{
    rt_hwtimer_mode_t mode;

    timer_dev = rt_device_find(DEMO_TIMER_DEV);

    if (timer_dev == RT_NULL)
    {
        PRT_Printf("find device %s failed\n", DEMO_TIMER_DEV);
        return;
    }

    PRT_Printf("find device %s success\n", DEMO_TIMER_DEV);

    rt_device_open(timer_dev, RT_DEVICE_OFLAG_RDWR);

    /* 设置超时回调函数 */
    rt_device_set_rx_indicate(timer_dev, timeout_callback);

    /* 设置为周期性定时器 */
    mode = HWTIMER_MODE_PERIOD;
    rt_device_control(timer_dev, HWTIMER_CTRL_MODE_SET, &mode);

    /* 设置定时器超时值，并启动定时器 */
    rt_hwtimerval_t tv;
    tv.sec = 0;
    tv.usec = 500000;
    rt_device_write(timer_dev, 0, &tv, sizeof(tv));

    PRT_TaskDelay(10050);

    rt_device_close(timer_dev);
}
