/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-21     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>

static int input_pin;
static int output_pin;
static int intr_cnt = 0;
static rt_device_t timer_dev = RT_NULL;

static void input_callback(void *parameter)
{
    intr_cnt++;
    // PRT_Printf("Input intr = %d\n", intr_cnt++);
}

static rt_err_t timeout_callback(rt_device_t dev, rt_size_t size)
{
    static U32 val = 0;

    rt_pin_write(output_pin, ++val & 1);

    return 0;
}

void gpio_test()
{
    PRT_Printf("GPIO Demo\n");

    input_pin = rt_pin_get("P2.8");
    output_pin = rt_pin_get("P5.10");

    rt_pin_mode(input_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(output_pin, PIN_MODE_OUTPUT);

    /* 设置中断类型及回调函数，注意E2000Q不支持双边沿触发 */
    rt_pin_attach_irq(input_pin, PIN_IRQ_MODE_FALLING, input_callback, 0);

    /* 使能中断 */
    rt_pin_irq_enable(input_pin, PIN_IRQ_ENABLE);

    rt_hwtimerval_t timeout_s;
    rt_hwtimer_mode_t mode;


    timer_dev = rt_device_find("timer0");
    if (timer_dev == RT_NULL)
    {
        PRT_Printf("timer device not found!\n");
        return;
    }

    rt_device_open(timer_dev, RT_DEVICE_OFLAG_RDWR);

    /* 设置超时回调函数 */
    rt_device_set_rx_indicate(timer_dev, timeout_callback);

    /* 设置为周期性定时器 */
    mode = HWTIMER_MODE_PERIOD;
    rt_device_control(timer_dev, HWTIMER_CTRL_MODE_SET, &mode);

    /* 设置定时器超时值，并启动定时器 */
    rt_hwtimerval_t tv;
    tv.sec = 0;
    tv.usec = 125;
    rt_device_write(timer_dev, 0, &tv, sizeof(tv));

    PRT_TaskDelay(50000);

    rt_device_close(timer_dev);
    PRT_Printf("Input intr = %d\n", intr_cnt);
    PRT_Printf("TIMER Demo End\n");
}
