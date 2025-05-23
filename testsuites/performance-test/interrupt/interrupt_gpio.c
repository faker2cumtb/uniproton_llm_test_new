/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-18     LuoYuncong   the first version
 */
#include <math.h>
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <prt_sys.h>
#include <prt_config.h>
#include <print.h>

static SemHandle isr_sem;
static bool print_flag = false;

/* 中断周期 us */
#define INTR_PERIOD 512.0f

/* 抖动误差us */
#define JITTER_ERROR 5.0f

static void input_callback(void *parameter)
{
    if (print_flag == false)
    {
        PRT_SemPost(isr_sem);
    }
}

void interrupt_main()
{
    float t1 = 0.0f;
    float t2 = INTR_PERIOD;
    float t_jitter = 0.0f;

    uint64_t last_clk = 0;
    uint32_t cnt = 0;
    uint32_t isr_cnt = 0;
    uint32_t err_flag = 0;

    int input_pin = rt_pin_get("P4.13");

    rt_pin_mode(input_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(input_pin, PIN_IRQ_MODE_FALLING, input_callback, 0);
    rt_pin_irq_enable(input_pin, PIN_IRQ_ENABLE);

    PRT_SemCreate(0, &isr_sem);

    while (1)
    {
        /* 每1000次中断一个周期，每个周期统计最后900个中断的t1取最大值 */
        if (PRT_SemPend(isr_sem, OS_WAIT_FOREVER) == OS_OK)
        {
            uint64_t delta = PRT_ClkGetCycleCount64() - last_clk;
            last_clk = PRT_ClkGetCycleCount64();

            isr_cnt++;

            t1 = 0.001f * PRT_ClkCycle2Ns(delta);
            float temp = fabs(t1 - t2);

            if (isr_cnt > 100)
            {
                if (temp > t_jitter)
                {
                    t_jitter = temp;
                }

                if (t_jitter > 5.0f)
                {
                    err_flag = 1;
                }
            }

            if (isr_cnt > 1000)
            {
                print_flag = true;

                PRT_Printf("[%d] t1 = %.1f us, t_jitter = %.1f us ", ++cnt, t1, t_jitter);

                if (err_flag == 1)
                {
                    PRT_Printf(", failed!\n");
                }
                else
                {
                    PRT_Printf("\n");
                }

                isr_cnt = 0;
                t_jitter = 0.0f;
                err_flag = 0;

                print_flag = false;
            }
        }
    }
}
