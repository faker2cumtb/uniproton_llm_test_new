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
#include <demo.h>

static int input_pin;
static int output_pin;
static int count = 20;

static void input_callback(void *parameter)
{
    static int intr_cnt = 0;

    if(count > 0)
    {
        PRT_Printf("Input intr = %d\n", intr_cnt++);
        count--;
    }
}

void gpio_demo()
{
    input_pin = rt_pin_get(DEMO_GPIO_INPUT_PIN);
    output_pin = rt_pin_get(DEMO_GPIO_OUTPUT_PIN);

    rt_pin_mode(input_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(output_pin, PIN_MODE_OUTPUT);

    rt_pin_attach_irq(input_pin, DEMO_GPIO_IRQ_MODE, input_callback, 0);

    /* 使能中断 */
    rt_pin_irq_enable(input_pin, PIN_IRQ_ENABLE);

    while (count)
    {
        rt_pin_write(output_pin, PIN_HIGH);
        PRT_TaskDelay(100);

        rt_pin_write(output_pin, PIN_LOW);
        PRT_TaskDelay(100);
    }

    rt_pin_irq_enable(input_pin, PIN_IRQ_DISABLE);
}
