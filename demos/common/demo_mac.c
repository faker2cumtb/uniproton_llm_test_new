/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-14     LuoYuncong   the first version
 * 2024-08-30     WuPeifeng    Add function implementation
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <demo.h>

static rt_device_t mac_dev;
static SemHandle mac_tx_sem;
static SemHandle mac_rx_sem;

/* 接收成功信号量 */
static rt_err_t mac_rx_indicate(rt_device_t dev, rt_size_t size)
{
    PRT_SemPost(mac_rx_sem);

    return RT_EOK;
}

/* 发送成功信号量 */
static rt_err_t mac_tx_complete(rt_device_t dev, void *buffer)
{
    PRT_SemPost(mac_tx_sem);

    return RT_EOK;
}

void mac_demo()
{
    int link_status = -1;
    int link_status_temp = 0;
    mac_dev = rt_device_find(DEMO_MAC_DEV);
    if (mac_dev == RT_NULL)
    {
        PRT_Printf("find device %s failed\n", DEMO_MAC_DEV);
        return;
    }

    PRT_Printf("find device %s success\n", DEMO_MAC_DEV);

    PRT_SemCreate(16, &mac_tx_sem);
    PRT_SemCreate(0, &mac_rx_sem);

    /* 设置接收通知回调 */
    rt_device_set_rx_indicate(mac_dev, mac_rx_indicate);

    /* 设置发送成功回调 */
    rt_device_set_tx_complete(mac_dev, mac_tx_complete);

    /* 注意回调函数以及mac地址要在open之前设置好 */
    if (rt_device_open(mac_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        PRT_Printf("xmac device open failed with \n");
        return;
    }

    /* 此例程仅作演示，当网口超过1秒接收不到数据时，会主动发送一包测试数据 */
    rt_size_t size;
    rt_uint8_t rx_buf[1500];
    // udp 数据包 192.168.123.10发到192.168.123.100， 端口5002
    rt_uint8_t tx_buf[] = {0x44, 0xef, 0xbf, 0x49, 0x3e, 0x7e, 0x98, 0xe,
                           0x24, 0x0, 0x11, 0x0, 0x8, 0x0, 0x45, 0x0,
                           0x0, 0x3d, 0x0, 0x3, 0x0, 0x0, 0xff, 0x11,
                           0x43, 0xed, 0xc0, 0xa8, 0x7b, 0xa, 0xc0, 0xa8,
                           0x7b, 0x64, 0x13, 0x89, 0x13, 0x8a, 0x0, 0x29,
                           0xb3, 0xb5, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69,
                           0x73, 0x20, 0x61, 0x20, 0x64, 0x61, 0x74, 0x61,
                           0x20, 0x66, 0x72, 0x6f, 0x6d, 0x20, 0x55, 0x64,
                           0x70, 0x20, 0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74,
                           0x21, 0x0a, 0x00};

    while (1)
    {
        while (PRT_SemPend(mac_rx_sem, 1000) == OS_OK)
        {
            while ((size = rt_device_read(mac_dev, 0, rx_buf, sizeof(rx_buf))) > 0)
            {
                PRT_Printf("xmac recv : %d bytes\n", size);
            }

            continue;
        }

        if (PRT_SemPend(mac_tx_sem, OS_WAIT_FOREVER) == OS_OK)
        {
            size = rt_device_write(mac_dev, 0, tx_buf, sizeof(tx_buf));
            PRT_Printf("xmac send %s\n", size == sizeof(tx_buf) ? "success" : "failed");
        }

#if defined(DEMO_MAC_LINK_STATUS)
        // 监测连接状态并在连接后进行网速的自动协商
        rt_device_control(mac_dev, RT_DEVICE_CTRL_NETIF_LINKSTATE, &link_status_temp);
        if (link_status != link_status_temp)
        {
            link_status = link_status_temp;
            if (link_status == 1)
            {
                PRT_Printf("link up\n");
            }
            else
            {
                PRT_Printf("link down\n");
            }
        }
#endif
    }
}
