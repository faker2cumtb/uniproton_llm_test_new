/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-23     huajian   the first version
 * 驱动层仅实现了发送阻塞，接收非阻塞 
 * 验证多任务场景下串口是否正常收发
 * 在rt_device_control   config 里面修改 波特率等配置
 */
#include <rtdevice.h>
#include <string.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <prt_sem.h>
#include <print.h>
#include <demo.h>
#include "prt_mem.h"

static rt_device_t uart_dev;
static SemHandle uart_sem;

/* 接收回调函数 */
static rt_err_t uart_rx_indicate(rt_device_t dev, rt_size_t size)
{
    PRT_SemPost(uart_sem);
    return RT_EOK;
}

void uart_test()
{
    const char *promt = "\r\nUniProton abcdefghffaas1234%^$^&*&* # ";
    char ch;

    PRT_Printf("UART Demo\n");

    uart_dev = rt_device_find("mio_uart14");
    if(uart_dev == RT_NULL)
    {
        PRT_Printf("uart device not found\n");
        return;
    }

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_115200;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.parity = PARITY_NONE;
    rt_device_control(uart_dev, RT_DEVICE_CTRL_CONFIG, &config);

    PRT_SemCreate(0, &uart_sem);

    /* 驱动层仅实现了发送阻塞，接收非阻塞 */
    rt_device_open(uart_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);

    /* 设置接收回调 */
    rt_device_set_rx_indicate(uart_dev, uart_rx_indicate);

    /* 发送提示信息 */
    rt_device_write(uart_dev, 0, promt, strlen(promt));

    while (1)
    {
        /* 等待接收数据 */
        PRT_SemPend(uart_sem, OS_WAIT_FOREVER);

        while(rt_device_read(uart_dev, 0, &ch, 1) == 1)
        {
            if( ch == '\r' || ch == '\n' )
            {
                rt_device_write(uart_dev, 0, promt, strlen(promt));
                continue;
            }

            if(ch >= 32 && ch <= 126)
            {
                rt_device_write(uart_dev, 0, &ch, 1);
            }
        }
    }
}

U32 create_task(void *pfunc, TskPrior prio, char *name)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle handle;

    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)pfunc;
    param.taskPrio = prio;

    param.name = name;
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&handle, &param);
    if (ret)
    {
        return ret;
    }

    ret = PRT_TaskResume(handle);
    if (ret)
    {
        return ret;
    }

    PRT_Printf("==========Create %s Success=========\n", name);
    return OS_OK;
}

void cpu_stress()
{
    sleep(1);
    while (true)
    {
        for (int i = 0; i < 1000; ++i)
        {
            for (int j = 1; j < 1000; ++j)
            {
                volatile double f = (double)(i + 0.2) * (double)(j + 0.5);
                (void)f;
            }
        }
        PRT_TaskDelay(1); // 让出CPU
    }
}

void uart_demo()
{
    PRT_Printf("==========test start=========\n");
    create_task(cpu_stress, 10, "cpu_stress");
    create_task(uart_test, 20, "uart_test");
    sleep(1000);
}
