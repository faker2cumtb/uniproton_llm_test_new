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

static rt_device_t timer_dev = RT_NULL;

static rt_err_t timeout_callback(rt_device_t dev, rt_size_t size)
{
    rt_hwtimerval_t tv;

    rt_device_read(timer_dev, 0, &tv, sizeof(tv));

    PRT_Printf("timeout = %d.%06d s\n", tv.sec, tv.usec);

    return 0;
}

int timer_diffperiod_test()
{
    rt_hwtimer_mode_t mode;

#ifdef GUEST_OS_JAILHOUSE
    timer_dev = rt_device_find("timer0");
#else
    timer_dev = rt_device_find("timer1");
#endif
    if (timer_dev == RT_NULL)
    {
        PRT_Printf("timer device not found!\n");
        return 1;
    }

    if (rt_device_open(timer_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        PRT_Printf("Failed to open timer device!\n");
        return 1;  
    }

    /* 设置超时回调函数 */
    if (rt_device_set_rx_indicate(timer_dev, timeout_callback) != RT_EOK)
    {
        PRT_Printf("Failed to set RX indicate!\n");
        rt_device_close(timer_dev);
        return 1;  
    }

    /* 设置为周期性定时器 */
    mode = HWTIMER_MODE_PERIOD;
    if (rt_device_control(timer_dev, HWTIMER_CTRL_MODE_SET, &mode) != RT_EOK)
    {
        PRT_Printf("Failed to set timer mode!\n");
        rt_device_close(timer_dev);
        return 1;  
    }

    /* 定义不同的超时值 */
    rt_hwtimerval_t periods[] = {
        {0, 10},     // 10微秒
        {0, 100},    // 100微秒
        {0, 1000},   // 1毫秒
        {1, 0},      // 1秒
    };

    for (int i = 0; i < sizeof(periods) / sizeof(periods[0]); i++)
    {
        // 设置定时器超时值，并启动定时器
        if (rt_device_write(timer_dev, 0, &periods[i], sizeof(periods[i])) != sizeof(periods[i]))
        {
            PRT_Printf("Failed to write timer value for period %d microseconds!\n", periods[i].usec);
            rt_device_close(timer_dev);
            return 1;  // 返回 5 表示写入值失败
        }

        // 延迟一段时间以便观察超时回调
        PRT_TaskDelay(10000); // 延迟1毫秒*10000
    }

    rt_device_close(timer_dev);

    return 0;
}
