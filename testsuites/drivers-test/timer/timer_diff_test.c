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

static rt_device_t timer_dev1 = RT_NULL;
static rt_device_t timer_dev2 = RT_NULL;
static rt_device_t timer_dev3 = RT_NULL;

static volatile int timer1_counter = 0;
static volatile int timer2_counter = 0;
static volatile int timer3_counter = 0;

static rt_err_t timeout_callback(rt_device_t dev, rt_size_t size)
{
    rt_hwtimerval_t tv;

    rt_device_read(dev, 0, &tv, sizeof(tv));

    return 0;
}

int timer_diff_test()
{
    rt_hwtimer_mode_t mode;

    timer_dev1 = rt_device_find("timer0");
    timer_dev2 = rt_device_find("timer1");
    timer_dev3 = rt_device_find("timer2");

    if (timer_dev1 == RT_NULL)
    {
        PRT_Printf("timer1 device not found!\n");
        return 1;
    }

    if (rt_device_open(timer_dev1, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        PRT_Printf("Failed to open timer device!\n");
        return 1; 
    }

    /* 设置超时回调函数 */
    if (rt_device_set_rx_indicate(timer_dev1, timeout_callback) != RT_EOK)
    {
        PRT_Printf("Failed to set RX indicate!\n");
        rt_device_close(timer_dev1);
        return 1;
    }

    /* 设置为周期性定时器 */
    mode = HWTIMER_MODE_PERIOD;
    if (rt_device_control(timer_dev1, HWTIMER_CTRL_MODE_SET, &mode) != RT_EOK)
    {
        PRT_Printf("Failed to set timer mode!\n");
        rt_device_close(timer_dev1);
        return 1; 
    }

    /* 设置定时器超时值，并启动定时器 */
    rt_hwtimerval_t tv1;
    tv1.sec = 0;
    tv1.usec = 100;
    if (rt_device_write(timer_dev1, 0, &tv1, sizeof(tv1)) != sizeof(tv1))
    {
        PRT_Printf("Failed to write timer value!\n");
        rt_device_close(timer_dev1);
        return 1;  
    }

    PRT_TaskDelay(5000);

    rt_device_close(timer_dev1);

    if (timer_dev2 == RT_NULL)
    {
        PRT_Printf("timer2 device not found!\n");
        return 1;
    }

    if (rt_device_open(timer_dev2, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        PRT_Printf("Failed to open timer device!\n");
        return 1; 
    }

    /* 设置超时回调函数 */
    if (rt_device_set_rx_indicate(timer_dev2, timeout_callback) != RT_EOK)
    {
        PRT_Printf("Failed to set RX indicate!\n");
        rt_device_close(timer_dev2);
        return 1;
    }

    /* 设置为周期性定时器 */
    mode = HWTIMER_MODE_PERIOD;
    if (rt_device_control(timer_dev2, HWTIMER_CTRL_MODE_SET, &mode) != RT_EOK)
    {
        PRT_Printf("Failed to set timer mode!\n");
        rt_device_close(timer_dev2);
        return 1; 
    }

    /* 设置定时器超时值，并启动定时器 */
    rt_hwtimerval_t tv2;
    tv2.sec = 0;
    tv2.usec = 1000;
    if (rt_device_write(timer_dev2, 0, &tv2, sizeof(tv2)) != sizeof(tv2))
    {
        PRT_Printf("Failed to write timer value!\n");
        rt_device_close(timer_dev2);
        return 1;  
    }

    PRT_TaskDelay(5000);

    rt_device_close(timer_dev2);

    if (timer_dev3 == RT_NULL)
    {
        PRT_Printf("timer3 device not found!\n");
        return 1;
    }

    if (rt_device_open(timer_dev3, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        PRT_Printf("Failed to open timer device!\n");
        return 1; 
    }

    /* 设置超时回调函数 */
    if (rt_device_set_rx_indicate(timer_dev3, timeout_callback) != RT_EOK)
    {
        PRT_Printf("Failed to set RX indicate!\n");
        rt_device_close(timer_dev3);
        return 1;
    }

    /* 设置为单次触发 */
    mode = HWTIMER_MODE_ONESHOT;
    if (rt_device_control(timer_dev3, HWTIMER_CTRL_MODE_SET, &mode) != RT_EOK)
    {
        PRT_Printf("Failed to set timer mode!\n");
        rt_device_close(timer_dev3);
        return 1; 
    }

    /* 设置定时器超时值，并启动定时器 */
    rt_hwtimerval_t tv3;
    tv3.sec = 0;
    tv3.usec = 1000;
    if (rt_device_write(timer_dev3, 0, &tv3, sizeof(tv3)) != sizeof(tv3))
    {
        PRT_Printf("Failed to write timer value!\n");
        rt_device_close(timer_dev3);
        return 1;  
    }

    PRT_TaskDelay(1000);

    rt_device_close(timer_dev3);

    return 0;
}
