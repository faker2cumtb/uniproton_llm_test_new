/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-06     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <demo.h>

static rt_device_t can_dev;

rt_err_t can_set_filter(rt_device_t dev)
{
    struct rt_can_filter_item items[] = {

        /* 列表模式：仅接收ID为0x00的标准帧 */
        RT_CAN_FILTER_ITEM_INIT(0x00, RT_CAN_STDID, RT_CAN_DTR, 0, 0x00, RT_NULL, RT_NULL),

        /* 列表模式：仅接收ID为0x01的标准帧 */
        RT_CAN_FILTER_ITEM_INIT(0x01, RT_CAN_STDID, RT_CAN_DTR, 0, 0x00, RT_NULL, RT_NULL),

        /* 列表模式：仅接收ID为0x02的标准帧 */
        RT_CAN_FILTER_ITEM_INIT(0x02, RT_CAN_STDID, RT_CAN_DTR, 0, 0x00, RT_NULL, RT_NULL),

        /* 掩码模式：仅接收ID以0x15结尾的扩展帧 */
        RT_CAN_FILTER_ITEM_INIT(0x15, RT_CAN_EXTID, RT_CAN_DTR, 1, 0x0f, RT_NULL, RT_NULL),

    };

    struct rt_can_filter_config cfg = {sizeof(items) / sizeof(items[0]), 1, items};

    return rt_device_control(can_dev, RT_CAN_CMD_SET_FILTER, &cfg);
}

void can_demo()
{
    can_dev = rt_device_find(DEMO_CAN_DEV);
    if (can_dev == RT_NULL)
    {
        PRT_Printf("find device %s failed\n", DEMO_CAN_DEV);
        return;
    }

    PRT_Printf("find device %s success\n", DEMO_CAN_DEV);

    rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);

    rt_device_control(can_dev, RT_CAN_CMD_SET_BAUD, (void *)DEMO_CAN_BAUD);

    /* 设置CAN模式，正常模式 */
    rt_device_control(can_dev, RT_CAN_CMD_SET_MODE, (void *)DEMO_CAN_MODE);

    /* 过滤位设置，如果不设置过滤位，则无法接收数据 */
    can_set_filter(can_dev);

    while (1)
    {
        struct rt_can_msg msg;

        /* 因驱动实现问题，此处必须设为-1，表示直接从uselist链表读取数据 */
        msg.hdr_index = -1;

        if (rt_device_read(can_dev, 0, &msg, sizeof(msg)) != 0)
        {
            rt_device_write(can_dev, 0, &msg, sizeof(msg));
        }
    }
}
