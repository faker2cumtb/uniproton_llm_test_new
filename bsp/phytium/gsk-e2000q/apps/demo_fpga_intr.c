/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-24     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <demo.h>

static int int1_pin;
static int int2_pin;
static int led_pin;
static uint32_t int1_count = 0;
static uint32_t int2_count = 0;

static SemHandle int1_sem;

static void int1_callback(void *parameter)
{
    int1_count++;

    /* 每中断500次，发送一个信号量通知线程打印输出 */
    if (int1_count % 500 == 0)
    {
        PRT_SemPost(int1_sem);
    }
}

static void int2_callback(void *parameter)
{
    int2_count++;
}

void fpga_intr_demo()
{
    PRT_SemCreate(0, &int1_sem);

    /* 查找GPIO引脚 */
    int1_pin = rt_pin_get("P1.11");
    int2_pin = rt_pin_get("P1.12");
    led_pin = rt_pin_get("P2.2");

    /* 设置输入输出模式 */
    rt_pin_mode(int1_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(int2_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(led_pin, PIN_MODE_OUTPUT);

    /* 设置回调函数 */
    rt_pin_attach_irq(int1_pin, DEMO_GPIO_IRQ_MODE, int1_callback, 0);
    rt_pin_attach_irq(int2_pin, DEMO_GPIO_IRQ_MODE, int1_callback, 0);

    /* 使能中断 */
    rt_pin_irq_enable(int1_pin, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(int2_pin, PIN_IRQ_ENABLE);

    uint64_t last_clk = 0;
    uint64_t delta;

    for (int cnt = 0; cnt < 100; cnt++)
    {
        /* 等待int1接收事件 */
        PRT_SemPend(int1_sem, OS_WAIT_FOREVER);

        delta = PRT_ClkGetCycleCount64() - last_clk;
        last_clk = PRT_ClkGetCycleCount64();

        if (cnt > 0)
        {
            PRT_Printf("[%d] int1_count: %d, int2_count: %d, delta: %lld us\n", cnt, int1_count, int2_count, PRT_ClkCycle2Us(delta));

            /* led灯翻转 */
            if (cnt & 1)
            {
                rt_pin_write(led_pin, PIN_LOW);
            }
            else
            {
                rt_pin_write(led_pin, PIN_HIGH);
            }
        }
    }
}
