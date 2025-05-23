/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-23     huajian   the first version
 * 驱动层仅实现了发送阻塞，接收非阻塞 
 * 验证设置非支持模式正常报错
 */
#include <rtdevice.h>
#include <string.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <prt_sem.h>
#include <print.h>

static rt_device_t uart_dev;
static SemHandle uart_sem;

void uart_demo()
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

    PRT_SemCreate(0, &uart_sem);

    /* 驱动层仅实现了发送阻塞，接收非阻塞 */
    if(rt_device_open(uart_dev, RT_DEVICE_FLAG_RX_BLOCKING | RT_DEVICE_FLAG_TX_NON_BLOCKING)){
        PRT_Printf("打开设备失败，符合预期\n");
    } 

    PRT_Printf("打开设备成功，测试失败\n");
}
