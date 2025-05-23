/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-06     LuoYuncong   the first version
 * 2024-11-07     LuoYuncong   fix send and recv bug
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <hal_base.h>
#include "drv_can.h"

struct rk3588_can
{
    char *name;

    struct CAN_REG *reg;
    uint32_t baud_rate;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    eGPIO_bankId bank;
    uint32_t rx_pin;
    uint32_t tx_pin;
    ePINCTRL_configParam param;

    struct rt_can_device device;
    struct CANFD_CONFIG config;
};

#define _RK3588_CAN(_name, _reg, _baud_rate, _intr_num) \
    .name = (_name),                                          \
    .reg = (_reg),                                            \
    .baud_rate = (_baud_rate),                                \
    .intr_num = (_intr_num),                                  \
    .intr_prio = 8

#define _RK3588_CAN_PIN(_bank, _rx_pin, _tx_pin, _param) \
    .bank = (_bank),                                     \
    .rx_pin = (_rx_pin),                                 \
    .tx_pin = (_tx_pin),                                 \
    .param = (_param)

static struct rk3588_can can_obj[] = {
#if defined(BSP_USING_CAN0)
    {
        _RK3588_CAN("can0", CAN0, BSP_CAN0_BAUDRATE, 373),
#if defined(BSP_USING_CAN0_M0)
        _RK3588_CAN_PIN(GPIO_BANK0, GPIO_PIN_C0, GPIO_PIN_B7, PIN_CONFIG_MUX_FUNC11),
#elif defined(BSP_USING_CAN0_M1)
        _RK3588_CAN_PIN(GPIO_BANK4, GPIO_PIN_D5, GPIO_PIN_D4, PIN_CONFIG_MUX_FUNC9),
#else
#error "Please define CAN0 Module."
#endif
    },
#endif

#if defined(BSP_USING_CAN1)
    {
        _RK3588_CAN("can1", CAN1, BSP_CAN1_BAUDRATE, 374),
#if defined(BSP_USING_CAN1_M0)
        _RK3588_CAN_PIN(GPIO_BANK3, GPIO_PIN_B5, GPIO_PIN_B6, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_CAN1_M1)
        _RK3588_CAN_PIN(GPIO_BANK4, GPIO_PIN_B2, GPIO_PIN_B3, PIN_CONFIG_MUX_FUNC12),
#else
#error "Please define CAN1 Module."
#endif
    },
#endif

#if defined(BSP_USING_CAN2)
    {
        _RK3588_CAN("can2", CAN2, BSP_CAN2_BAUDRATE, 375),
#if defined(BSP_USING_CAN2_M0)
        _RK3588_CAN_PIN(GPIO_BANK3, GPIO_PIN_C4, GPIO_PIN_C5, PIN_CONFIG_MUX_FUNC9),
#elif defined(BSP_USING_CAN2_M1)
        _RK3588_CAN_PIN(GPIO_BANK0, GPIO_PIN_D4, GPIO_PIN_D5, PIN_CONFIG_MUX_FUNC10),
#else
#error "Please define CAN2 Module."
#endif
    },
#endif
};

static void rk3588_can_isr(uintptr_t arg)
{
    struct rk3588_can *drv_can = (struct rk3588_can *)arg;
    uint32_t intr_status = HAL_CANFD_GetInterrupt(drv_can->reg);

    if (intr_status & CAN_INT_RX_FINISH_INT_MASK)
    {
        rt_hw_can_isr(&drv_can->device, RT_CAN_EVENT_RX_IND);
    }

    if (intr_status & CAN_INT_TX_FINISH_INT_MASK)
    {
        rt_hw_can_isr(&drv_can->device, RT_CAN_EVENT_TX_DONE);
    }
}

int rk3588_can_get_bps(uint32_t baud_rate)
{
    switch (baud_rate)
    {
    case CAN1MBaud:
        return CANFD_BPS_1MBAUD;
    case CAN500kBaud:
        return CANFD_BPS_500KBAUD;
    case CAN250kBaud:
        return CANFD_BPS_250KBAUD;
    case CAN125kBaud:
        return CANFD_BPS_125KBAUD;
    case CAN100kBaud:
        return CANFD_BPS_100KBAUD;
    default:
        return -1;
    }
}

static rt_err_t rk3588_can_config(struct rt_can_device *can, struct can_configure *cfg)
{
    RT_ASSERT(can);
    RT_ASSERT(cfg);

    struct rk3588_can *drv_can = (struct rk3588_can *)can->parent.user_data;
    struct CANFD_CONFIG *config = &drv_can->config;

    int bps = rk3588_can_get_bps(cfg->baud_rate);
    if (bps == -1)
    {
        return -RT_ERROR;
    }
    config->bps = bps;

    HAL_CANFD_Stop(drv_can->reg);
    HAL_CANFD_Init(drv_can->reg, config);
    HAL_CANFD_Start(drv_can->reg);

    return RT_EOK;
}

static rt_err_t rk3588_can_control(struct rt_can_device *can, int cmd, void *arg)
{
    RT_ASSERT(can);

    struct rk3588_can *drv_can = (struct rk3588_can *)can->parent.user_data;
    struct CANFD_CONFIG *config = &drv_can->config;
    uintptr_t argval = (uintptr_t)arg;

#ifdef RT_CAN_USING_HDR
    struct rt_can_filter_config *filter_cfg;
#endif

    switch (cmd)
    {
    case RT_DEVICE_CTRL_SET_INT:
        break;

    case RT_CAN_CMD_SET_MODE:
        switch (argval)
        {
        case RT_CAN_MODE_NORMAL:
            config->canfdMode = 0;
            drv_can->device.config.mode = RT_CAN_MODE_NORMAL;
            break;
        case RT_CAN_MODE_LISTEN:
            config->canfdMode = CANFD_MODE_LISTENONLY;
            drv_can->device.config.mode = RT_CAN_MODE_LISTEN;
            break;
        case RT_CAN_MODE_LOOPBACK:
            config->canfdMode = CANFD_MODE_LOOPBACK;
            drv_can->device.config.mode = RT_CAN_MODE_LOOPBACK;
            break;
        case RT_CAN_MODE_LOOPBACKANLISTEN:
            config->canfdMode = CANFD_MODE_LISTENONLY | CANFD_MODE_LOOPBACK;
            drv_can->device.config.mode = RT_CAN_MODE_LOOPBACKANLISTEN;
            break;
        default:
            return -RT_ERROR;
        }

        HAL_CANFD_Stop(drv_can->reg);
        HAL_CANFD_Init(drv_can->reg, config);
        HAL_CANFD_Start(drv_can->reg);
        break;

    case RT_CAN_CMD_SET_BAUD:
        if (argval != drv_can->device.config.baud_rate)
        {
            int bps = rk3588_can_get_bps(argval);
            if (bps == -1)
            {
                return -RT_ERROR;
            }

            config->bps = bps;
            drv_can->device.config.baud_rate = argval;
            HAL_CANFD_Stop(drv_can->reg);
            HAL_CANFD_Init(drv_can->reg, config);
            HAL_CANFD_Start(drv_can->reg);
        }
        break;

    case RT_CAN_CMD_SET_FILTER:
#ifdef RT_CAN_USING_HDR
        filter_cfg = (struct rt_can_filter_config *)arg;
        if (filter_cfg->count > drv_can->device.config.maxhdr)
        {
            return -RT_ERROR;
        }

        for (int i = 0; i < filter_cfg->count; i++)
        {
            if (filter_cfg->items[i].mode == 0)
            {
                config->canfdFilterMask[i] = 0;
                config->canfdFilterId[i] = filter_cfg->items[i].id;
            }
            else
            {
                config->canfdFilterMask[i] = ~filter_cfg->items[i].mask;
                config->canfdFilterId[i] = filter_cfg->items[i].id;
            }
        }
        HAL_CANFD_Init(drv_can->reg, config);
        HAL_CANFD_Start(drv_can->reg);
#endif
        break;
    }

    return RT_EOK;
}

rt_size_t rk3588_can_sendmsg(struct rt_can_device *can, const void *buf, rt_uint32_t boxno)
{
    RT_ASSERT(can);
    RT_ASSERT(buf);

    struct rk3588_can *drv_can = (struct rk3588_can *)can->parent.user_data;
    struct rt_can_msg *pmsg = (struct rt_can_msg *)buf;
    struct CANFD_MSG send_msg = {0};

    if (pmsg->ide == RT_CAN_STDID)
    {
        send_msg.stdId = pmsg->id;
        send_msg.ide = CANFD_ID_STANDARD;
    }
    else
    {
        send_msg.extId = pmsg->id;
        send_msg.ide = CANFD_ID_EXTENDED;
    }

    if (pmsg->rtr == RT_CAN_DTR)
    {
        send_msg.rtr = CANFD_RTR_DATA;
    }
    else
    {
        send_msg.rtr = CANFD_RTR_REMOTE;
    }

    send_msg.dlc = pmsg->len;
    if (send_msg.dlc > 8)
    {
        send_msg.dlc = 8;
    }

    memcpy(send_msg.data, pmsg->data, send_msg.dlc);

    if (HAL_CANFD_Transmit(drv_can->reg, &send_msg) != HAL_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_size_t rk3588_can_recvmsg(struct rt_can_device *can, void *buf, rt_uint32_t boxno)
{
    RT_ASSERT(can);
    RT_ASSERT(buf);

    struct rk3588_can *drv_can = (struct rk3588_can *)can->parent.user_data;
    struct rt_can_msg *pmsg = (struct rt_can_msg *)buf;
    struct CANFD_MSG recv_msg;

    if (HAL_CANFD_Receive(drv_can->reg, &recv_msg) != HAL_OK)
    {
        return -RT_ERROR;
    }

    memset(pmsg, 0, sizeof(struct rt_can_msg));
    if (recv_msg.ide == CANFD_ID_STANDARD)
    {
        pmsg->ide = RT_CAN_STDID;
        pmsg->id = recv_msg.stdId;
    }
    else
    {
        pmsg->ide = RT_CAN_EXTID;
        pmsg->id = recv_msg.extId;
    }

    if (recv_msg.rtr == CANFD_RTR_DATA)
    {
        pmsg->rtr = RT_CAN_DTR;
    }
    else
    {
        pmsg->rtr = RT_CAN_RTR;
    }

    pmsg->len = (recv_msg.dlc > 8) ? 8 : recv_msg.dlc;
    memcpy(pmsg->data, recv_msg.data, pmsg->len);

    pmsg->hdr_index = 0;

    return RT_EOK;
}

static const struct rt_can_ops rk3588_can_ops = {
    rk3588_can_config,
    rk3588_can_control,
    rk3588_can_sendmsg,
    rk3588_can_recvmsg,
};

rt_err_t rk3588_can_gpio_init(struct rk3588_can *drv_can)
{
    if (HAL_PINCTRL_SetIOMUX(drv_can->bank, drv_can->rx_pin | drv_can->tx_pin, drv_can->param) != HAL_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rk3588_can_config_init(struct rk3588_can *drv_can)
{
    struct CANFD_CONFIG *config = &drv_can->config;
    struct can_configure *bus_config = &drv_can->device.config;

    memset(config, 0, sizeof(struct CANFD_CONFIG));
    for (int i = 0; i < 6; i++)
    {
        config->canfdFilterMask[i] = 0x1fffffff;
    }

    config->bps = rk3588_can_get_bps(drv_can->baud_rate);
    if (config->bps == -1)
    {
        config->bps = CANFD_BPS_1MBAUD;
        drv_can->baud_rate = CAN1MBaud;
    }

    memset(bus_config, 0, sizeof(struct can_configure));
    bus_config->baud_rate = drv_can->baud_rate;
    bus_config->msgboxsz = 12;
    bus_config->sndboxnumber = 1; // not change
    bus_config->mode = RT_CAN_MODE_NORMAL;
    bus_config->privmode = RT_CAN_MODE_NOPRIV;
    bus_config->ticks = 1000;
#ifdef RT_CAN_USING_HDR
    bus_config->maxhdr = 6;
#endif

    return RT_EOK;
}

rt_err_t rk3588_can_intr_init(struct rk3588_can *drv_can)
{
    U32 ret;

    ret = PRT_HwiSetAttr(drv_can->intr_num, drv_can->intr_prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    ret = PRT_HwiCreate(drv_can->intr_num, rk3588_can_isr, (uintptr_t)drv_can);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    PRT_HwiSetRouter(drv_can->intr_num);
    ret = PRT_HwiEnable(drv_can->intr_num);
    if (ret != OS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

int drv_can_init()
{
    for (int i = 0; i < sizeof(can_obj) / sizeof(can_obj[0]); i++)
    {
        if (rk3588_can_gpio_init(&can_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_can_config_init(&can_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_can_intr_init(&can_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rt_hw_can_register(&can_obj[i].device, can_obj[i].name, &rk3588_can_ops, &can_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
