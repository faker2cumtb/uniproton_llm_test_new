/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-23     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <prt_sem.h>
#include <print.h>
#include <demo.h>

static rt_device_t uart_dev;
static SemHandle uart_sem;

/* 接收回调函数 */
static rt_err_t uart_rx_indicate(rt_device_t dev, rt_size_t size)
{
    PRT_SemPost(uart_sem);
    return RT_EOK;
}

void uart_demo()
{
    const char *promt = "\r\nUniProton # ";
    char ch;
    char buf[48];

    uart_dev = rt_device_find(DEMO_UART_DEV);
    if(uart_dev == RT_NULL)
    {
        PRT_Printf("find device %s failed\n", DEMO_UART_DEV);
        return;
    }

    PRT_Printf("find device %s\n", DEMO_UART_DEV);

    PRT_SemCreate(0, &uart_sem);

    /* 配置串口参数，如果不配置则系统默认为115200, 8N1 */
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = DEMO_UART_BAUD_RATE;
    config.data_bits = DEMO_UART_DATA_BITS;
    config.stop_bits = DEMO_UART_STOP_BITS;
    config.parity = DEMO_UART_PARITY;
    rt_device_control(uart_dev, RT_DEVICE_CTRL_CONFIG, &config);

    /* 发送阻塞，接收非阻塞 */
    rt_device_open(uart_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);

    /* 设置接收回调 */
    rt_device_set_rx_indicate(uart_dev, uart_rx_indicate);

    sprintf(buf, "\r\n========================");
    rt_device_write(uart_dev, 0, buf, strlen(buf));

    sprintf(buf, "\r\nThis is %s device, %d bps", DEMO_UART_DEV, DEMO_UART_BAUD_RATE);
    rt_device_write(uart_dev, 0, buf, strlen(buf));

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
