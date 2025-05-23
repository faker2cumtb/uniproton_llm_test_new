/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-04     LuoYuncong   the first version
 * 2024-11-07     LuoYuncong   fix send and recv bug
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <fiopad.h>
#include <fcan.h>
#include <fcan_hw.h>
#include "drv_common.h"
#include "drv_can.h"

struct e2000q_can
{
    char *name;

    FCanCtrl ctrl;
    s32 id;
    u32 baud_rate;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    u32 tx_reg_off;
    u32 rx_reg_off;
    FIOPadFunc func;

    struct rt_can_device device;
};

#define _E2000Q_CAN(_name, _id, _baud_rate, _tx_reg_off, _rx_reg_off, _func, _intr_num) \
    {                                                                                   \
        .name = (_name),                                                                \
        .id = (_id),                                                                    \
        .baud_rate = (_baud_rate),                                                      \
        .tx_reg_off = (_rx_reg_off),                                                    \
        .tx_reg_off = (_rx_reg_off),                                                    \
        .func = (_func),                                                                \
        .intr_num = (_intr_num),                                                        \
        .intr_prio = 6,                                                                 \
    }

static struct e2000q_can can_obj[] = {

#if defined(BSP_USING_CAN0)
    _E2000Q_CAN("can0",
                FCAN0_ID,
                BSP_CAN0_BAUDRATE,
                FIOPAD_A41_REG0_OFFSET,
                FIOPAD_A43_REG0_OFFSET,
                FIOPAD_FUNC0,
                FCAN0_IRQ_NUM),
#endif

#if defined(BSP_USING_CAN1)
    _E2000Q_CAN("can1",
                FCAN1_ID,
                BSP_CAN1_BAUDRATE,
                FIOPAD_A45_REG0_OFFSET,
                FIOPAD_C45_REG0_OFFSET,
                FIOPAD_FUNC0,
                FCAN1_IRQ_NUM),
#endif

};

static void e2000q_can_intr_entry(uintptr_t args)
{
    struct e2000q_can *drv_can = (struct e2000q_can *)args;

    FCanIntrHandler(0, &drv_can->ctrl);
}

static void e2000q_can_send_isr(void *args)
{
    struct e2000q_can *drv_can = (struct e2000q_can *)args;

    rt_hw_can_isr(&drv_can->device, RT_CAN_EVENT_TX_DONE);
}

static void e2000q_can_recv_isr(void *args)
{
    struct e2000q_can *drv_can = (struct e2000q_can *)args;

    rt_hw_can_isr(&drv_can->device, RT_CAN_EVENT_RX_IND);
}

static rt_err_t e2000q_can_init(struct rt_can_device *can, struct can_configure *cfg, struct rt_can_filter_config *filter)
{
    struct e2000q_can *drv_can = (struct e2000q_can *)can->parent.user_data;

    if (FCanCfgInitialize(&drv_can->ctrl, FCanLookupConfig(drv_can->id)) != FCAN_SUCCESS)
    {
        return -RT_ERROR;
    }

    FCanFdEnable(&drv_can->ctrl, FALSE);

    /* Work mode config */
    {
        switch (cfg->mode)
        {
        case RT_CAN_MODE_NORMAL:
            FCanSetMode(&drv_can->ctrl, FCAN_PROBE_NORMAL_MODE);
            break;
        case RT_CAN_MODE_LISTEN:
            FCanSetMode(&drv_can->ctrl, FCAN_PROBE_MONITOR_MODE);
            break;
        default:
            return -RT_ERROR;
        }
    }

    /* Baudrate config */
    {
        FCanBaudrateConfig arb_segment_config = {0};
        FCanBaudrateConfig data_segment_config = {0};

        arb_segment_config.baudrate = cfg->baud_rate;
        arb_segment_config.auto_calc = TRUE;
        arb_segment_config.segment = FCAN_ARB_SEGMENT;
        if (FCanBaudrateSet(&drv_can->ctrl, &arb_segment_config) != FCAN_SUCCESS)
        {
            return -RT_ERROR;
        }

        data_segment_config.baudrate = cfg->baud_rate;
        data_segment_config.auto_calc = TRUE;
        data_segment_config.segment = FCAN_DATA_SEGMENT;
        if (FCanBaudrateSet(&drv_can->ctrl, &data_segment_config) != FCAN_SUCCESS)
        {
            return -RT_ERROR;
        }
    }

    /* Filter config */
    {
        FCanIdMaskConfig id_mask;

        if (filter != RT_NULL)
        {
            if (filter->count > drv_can->device.config.maxhdr)
            {
                return -RT_ERROR;
            }

            for (int i = 0; i < filter->count; i++)
            {
                memset(&id_mask, 0, sizeof(id_mask));

                id_mask.filter_index = i;
                id_mask.id = filter->items[i].id;

                if (filter->items[i].mode == 1)
                {
                    id_mask.mask = ~filter->items[i].mask;
                }

                if (filter->items[i].ide == RT_CAN_EXTID)
                {
                    id_mask.type = EXTEND_FRAME;
                }

                FCanIdMaskFilterSet(&drv_can->ctrl, &id_mask);
            }
        }

        FCanIdMaskFilterEnable(&drv_can->ctrl);
    }

    /* Interrupt config */
    {
        FCanIntrEventConfig intr_event;

        memset(&intr_event, 0, sizeof(intr_event));
        intr_event.type = FCAN_INTR_EVENT_SEND;
        intr_event.handler = e2000q_can_send_isr;
        intr_event.param = drv_can;
        FCanRegisterInterruptHandler(&drv_can->ctrl, &intr_event);
        FCanInterruptEnable(&drv_can->ctrl, intr_event.type);

        memset(&intr_event, 0, sizeof(intr_event));
        intr_event.type = FCAN_INTR_EVENT_RECV;
        intr_event.handler = e2000q_can_recv_isr;
        intr_event.param = drv_can;
        FCanRegisterInterruptHandler(&drv_can->ctrl, &intr_event);
        FCanInterruptEnable(&drv_can->ctrl, intr_event.type);
    }

    FCanEnable(&drv_can->ctrl, TRUE);

    return RT_EOK;
}

static rt_err_t e2000q_can_config(struct rt_can_device *can, struct can_configure *cfg)
{
    return e2000q_can_init(can, cfg, RT_NULL);
}

static rt_err_t e2000q_can_control(struct rt_can_device *can, int cmd, void *arg)
{
    struct e2000q_can *drv_can = (struct e2000q_can *)can->parent.user_data;
    struct can_configure config = drv_can->device.config;
    uintptr_t argval = (uintptr_t)arg;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_SET_INT:
        break;

    case RT_CAN_CMD_SET_MODE:
        config.mode = argval;
        if (e2000q_can_init(can, &config, RT_NULL) != RT_EOK)
        {
            return -RT_ERROR;
        }
        drv_can->device.config.mode = argval;
        break;

    case RT_CAN_CMD_SET_BAUD:
        config.baud_rate = argval;
        if (e2000q_can_init(can, &config, RT_NULL) != RT_EOK)
        {
            return -RT_ERROR;
        }
        drv_can->device.config.baud_rate = argval;
        break;

    case RT_CAN_CMD_SET_FILTER:
#ifdef RT_CAN_USING_HDR
        if (e2000q_can_init(can, &config, (struct rt_can_filter_config *)arg) != RT_EOK)
        {
            return -RT_ERROR;
        }
#endif
        break;
    }

    return RT_EOK;
}

rt_size_t e2000q_can_sendmsg(struct rt_can_device *can, const void *buf, rt_uint32_t boxno)
{
    struct e2000q_can *drv_can = (struct e2000q_can *)can->parent.user_data;
    struct rt_can_msg *pmsg = (struct rt_can_msg *)buf;
    FCanFrame frame = {0};

    frame.canid = pmsg->id;

    if (pmsg->ide == RT_CAN_EXTID)
    {
        frame.canid &= CAN_EFF_MASK;
        frame.canid |= CAN_EFF_FLAG;
    }
    else
    {
        frame.canid &= CAN_SFF_MASK;
    }

    if (pmsg->rtr == RT_CAN_RTR)
    {
        frame.canid |= CAN_RTR_FLAG;
    }

    frame.candlc = (pmsg->len > 8) ? 8 : pmsg->len;

    memcpy(frame.data, pmsg->data, frame.candlc);

    if (FCanSend(&drv_can->ctrl, &frame) != FCAN_SUCCESS)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_size_t e2000q_can_recvmsg(struct rt_can_device *can, void *buf, rt_uint32_t boxno)
{
    struct e2000q_can *drv_can = (struct e2000q_can *)can->parent.user_data;
    struct rt_can_msg *pmsg = (struct rt_can_msg *)buf;
    FCanFrame frame = {0};

    if (FCanRecv(&drv_can->ctrl, &frame) != FCAN_SUCCESS)
    {
        return -RT_ERROR;
    }

    memset(pmsg, 0, sizeof(struct rt_can_msg));

    if (frame.canid & CAN_EFF_FLAG)
    {
        pmsg->ide = RT_CAN_EXTID;
        pmsg->id = frame.canid & CAN_EFF_MASK;
    }
    else
    {
        pmsg->ide = RT_CAN_STDID;
        pmsg->id = frame.canid & CAN_SFF_MASK;
    }

    if (frame.canid & CAN_RTR_FLAG)
    {
        pmsg->rtr = RT_CAN_RTR;
    }
    else
    {
        pmsg->rtr = RT_CAN_DTR;
    }

    pmsg->len = (frame.candlc > 8) ? 8 : frame.candlc;
    memcpy(pmsg->data, frame.data, pmsg->len);

    pmsg->hdr_index = 0;

    return RT_EOK;
}

static const struct rt_can_ops e2000q_can_ops = {
    e2000q_can_config,
    e2000q_can_control,
    e2000q_can_sendmsg,
    e2000q_can_recvmsg,
};

rt_err_t e2000q_can_gpio_init(struct e2000q_can *drv_can)
{
    FIOPadSetFunc(&iopad_ctrl, drv_can->tx_reg_off, drv_can->func);
    FIOPadSetFunc(&iopad_ctrl, drv_can->rx_reg_off, drv_can->func);

    return RT_EOK;
}

rt_err_t e2000q_can_config_init(struct e2000q_can *drv_can)
{
    struct can_configure *pcfg = &drv_can->device.config;

    memset(pcfg, 0, sizeof(struct can_configure));
    pcfg->baud_rate = drv_can->baud_rate;
    pcfg->msgboxsz = 12;
    pcfg->sndboxnumber = 1; //not change
    pcfg->mode = RT_CAN_MODE_NORMAL;
    pcfg->privmode = RT_CAN_MODE_NOPRIV;
    pcfg->ticks = 1000;

#ifdef RT_CAN_USING_HDR
    pcfg->maxhdr = FCAN_ACC_ID_REG_NUM;
#endif

    return RT_EOK;
}

rt_err_t e2000q_can_intr_init(struct e2000q_can *drv_can)
{
    if (PRT_HwiSetAttr(drv_can->intr_num, drv_can->intr_prio, OS_HWI_MODE_ENGROSS) != OS_OK)
    {
        return -RT_ERROR;
    }

    if (PRT_HwiCreate(drv_can->intr_num, e2000q_can_intr_entry, (uintptr_t)drv_can) != OS_OK)
    {
        return -RT_ERROR;
    }

    PRT_HwiSetRouter(drv_can->intr_num);

    if (PRT_HwiEnable(drv_can->intr_num) != OS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

int drv_can_init()
{
    for (int i = 0; i < sizeof(can_obj) / sizeof(can_obj[0]); i++)
    {
        if (e2000q_can_gpio_init(&can_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_can_config_init(&can_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (e2000q_can_intr_init(&can_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rt_hw_can_register(&can_obj[i].device, can_obj[i].name, &e2000q_can_ops, &can_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
