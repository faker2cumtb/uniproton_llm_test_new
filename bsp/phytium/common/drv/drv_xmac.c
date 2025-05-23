/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-19     LuoYuncong   the first version
 * 2024-08-30     WuPeifeng    Add function implementation
 */
#include "prt_buildef.h"
#include "print.h"
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include "drv_xmac.h"
#include "fxmac.h"
#include "fxmac_port.h"
#include "fxmac_port_nolwip.h"
#include <print.h>
#include <fiopad.h>
#include <drv_common.h>
#include <prt_task.h>
#include <prt_sys.h>
#include <prt_mem.h>

struct e2000q_xmac
{
    char *device_name;

    /* phy chip reset pin */
    // eGPIO_bankId phy_reset_bank;
    // struct GPIO_REG *phy_reset_gpio;
    // uint32_t phy_reset_pin;
    // ePINCTRL_configParam phy_reset_param;

    FXmacPort *instance_p;
    u32 instance_id;
    FXmacPortInterface interface;
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    u32 phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */

    struct rt_device device;
    TskHandle g_phyTskHandle;
};

static struct e2000q_xmac e2000q_xmac_obj[] = {
#if defined(BSP_USING_XMAC0)
    {
        .device_name = "xmac0",

        // .phy_reset_bank = GPIO_BANK3,
        // .phy_reset_gpio = GPIO3,
        // .phy_reset_pin = GPIO_PIN_B2,
        // .phy_reset_param = PIN_CONFIG_PUL_UP,

        .instance_id = FXMAC0_ID,
        .interface = FXMAC_PORT_INTERFACE_SGMII,
        .autonegotiation = 1,
        .phy_speed = FXMAC_SPEED_1000,
        .phy_duplex = 1,
    },
#endif

#if defined(BSP_USING_XMAC1)
    {
        .device_name = "xmac1",

        // .phy_reset_bank = GPIO_BANK3,
        // .phy_reset_gpio = GPIO3,
        // .phy_reset_pin = GPIO_PIN_B2,
        // .phy_reset_param = PIN_CONFIG_PUL_UP,

        .instance_id = FXMAC1_ID,
        .interface = FXMAC_PORT_INTERFACE_SGMII,
        .autonegotiation = 1,
        .phy_speed = FXMAC_SPEED_1000,
        .phy_duplex = 1,
    },
#endif

#if defined(BSP_USING_XMAC2)
    {
        .device_name = "xmac2",

        // .phy_reset_bank = GPIO_BANK3,
        // .phy_reset_gpio = GPIO3,
        // .phy_reset_pin = GPIO_PIN_B2,
        // .phy_reset_param = PIN_CONFIG_PUL_UP,

        .instance_id = FXMAC2_ID,
        .interface = FXMAC_PORT_INTERFACE_RGMII,
        .autonegotiation = 1,
        .phy_speed = FXMAC_SPEED_1000,
        .phy_duplex = 1,
    },
#endif

#if defined(BSP_USING_XMAC3)
    {
        .device_name = "xmac3",

        .instance_id = FXMAC3_ID,
        .interface = FXMAC_PORT_INTERFACE_RGMII,
        .autonegotiation = 1,
        .phy_speed = FXMAC_SPEED_100,
        .phy_duplex = 1,
    },
#endif
};

rt_err_t e2000q_xmac_config_init(struct e2000q_xmac *drv_xmac)
{
    FXmacPort *instance_p = NULL;

    drv_xmac->instance_p = FXmacPortGetInstancePointer(drv_xmac->instance_id);
    instance_p = drv_xmac->instance_p;

    instance_p->xmac_port_config.instance_id = drv_xmac->instance_id;
    instance_p->xmac_port_config.autonegotiation = drv_xmac->autonegotiation;
    instance_p->xmac_port_config.phy_speed = drv_xmac->phy_speed;
    instance_p->xmac_port_config.phy_duplex = drv_xmac->phy_duplex;
    instance_p->xmac_port_config.interface = drv_xmac->interface;

    return RT_EOK;
}

static rt_err_t e2000q_xmac_gpio_init(struct e2000q_xmac *drv_xmac)
{
    if(drv_xmac->instance_id == FXMAC3_ID){
        // rgmii1
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J37_REG0_OFFSET, FIOPAD_FUNC1); // 0x01CCU
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J39_REG0_OFFSET, FIOPAD_FUNC1); // 0x01D0U
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_G41_REG0_OFFSET, FIOPAD_FUNC1); // 0x01D4U
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_E43_REG0_OFFSET, FIOPAD_FUNC1); // 0x01D8U
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_L43_REG0_OFFSET, FIOPAD_FUNC1); // 0x01DCU
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_C43_REG0_OFFSET, FIOPAD_FUNC1); // 0x01E0U
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_E41_REG0_OFFSET, FIOPAD_FUNC1); // 0x01E4U
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_L45_REG0_OFFSET, FIOPAD_FUNC1); // 0x01E8U
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J43_REG0_OFFSET, FIOPAD_FUNC1); // 0x01ECU
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J41_REG0_OFFSET, FIOPAD_FUNC1); // 0x01F0U
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_L39_REG0_OFFSET, FIOPAD_FUNC1); // 0x01F4U
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_E37_REG0_OFFSET, FIOPAD_FUNC1); // 0x01F8U
        // MDC3 MDIO3
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_E35_REG0_OFFSET, FIOPAD_FUNC0);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_G35_REG0_OFFSET, FIOPAD_FUNC0);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_R51_REG0_OFFSET, FIOPAD_FUNC5);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_R49_REG0_OFFSET, FIOPAD_FUNC5);
    }
    return RT_EOK;
}

rt_err_t e2000q_xmac_init(rt_device_t dev)
{
    struct e2000q_xmac *drv_xmac = (struct e2000q_xmac *)dev->user_data;
    FXmacPortInit(drv_xmac->instance_p);
    drv_xmac->instance_p->device = dev;
    return RT_EOK;
}

rt_err_t e2000q_xmac_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct e2000q_xmac *drv_xmac = (struct e2000q_xmac *)dev->user_data;
    FXmacPortStart(drv_xmac->instance_p);
    return RT_EOK;
}

rt_err_t e2000q_xmac_close(rt_device_t dev)
{
    struct e2000q_xmac *drv_xmac = (struct e2000q_xmac *)dev->user_data;

    FXmacPortStop(drv_xmac->instance_p);

    return RT_EOK;
}

rt_size_t e2000q_xmac_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct e2000q_xmac *drv_xmac = (struct e2000q_xmac *)dev->user_data;
    struct fx_pbuf *rxp;

    rxp = FXmacPortRx(drv_xmac->instance_p);
    if (rxp != NULL)
    {
        int len = (rxp->len < size) ? rxp->len : size;

        memcpy(buffer, rxp->payload, len);
        fx_pbuf_free(rxp);
        return len;
    }

    return 0;
}

rt_size_t e2000q_xmac_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct e2000q_xmac *drv_xmac = (struct e2000q_xmac *)dev->user_data;
    struct fx_pbuf *txp;

    txp = fx_pbuf_alloc(size);
    if (txp == NULL) {
        PRT_Printf("fx_pbuf_alloc error\n");
        return 0;
    }
    memcpy(txp->payload, buffer, size);

    if (FXmacPortTx(drv_xmac->instance_p, txp) == 0)
    {
        return size;
    }

    return 0;
}

rt_err_t e2000q_xmac_control(rt_device_t dev, int cmd, void *arg)
{
    struct e2000q_xmac *drv_xmac = (struct e2000q_xmac *)dev->user_data;
    int intr_num = drv_xmac->instance_p->instance.config.queue_irq_num[0];

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        PRT_HwiDisable(intr_num);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        PRT_HwiEnable(intr_num);
        break;
    case RT_DEVICE_CTRL_NETIF_SETMAC:
        if(drv_xmac->instance_p->instance.config.base_address == 0) { /* No need to set MAC xmac init*/
            memcpy(drv_xmac->instance_p->hwaddr, arg, FXMAX_MAX_HARDWARE_ADDRESS_LENGTH);
            break;
        }
        if(FXmacSetMacAddress(&(drv_xmac->instance_p->instance), drv_xmac->instance_p->hwaddr, drv_xmac->instance_id) != FT_SUCCESS) {
            PRT_Printf("Set Mac Address err\n");
            return RT_ERROR;
        }else{
            memcpy(drv_xmac->instance_p->hwaddr, arg, FXMAX_MAX_HARDWARE_ADDRESS_LENGTH);
        }
        break;
    case RT_DEVICE_CTRL_NETIF_GETMAC:
        if(drv_xmac->instance_p->instance.config.base_address == 0) {
            memcpy(arg, drv_xmac->instance_p->hwaddr, FXMAX_MAX_HARDWARE_ADDRESS_LENGTH);
            break;
        }
        FXmacGetMacAddress(&drv_xmac->instance_p->instance, arg, drv_xmac->instance_id);
        break;
    case RT_DEVICE_CTRL_NETIF_LINKSTATE:
        if (FXmacPhyReconnect((FXmacPort *)drv_xmac->instance_p) == ETH_LINK_UP){
            *(int *)arg = 1;
        }else{
            *(int *)arg = 0;
        }
        break;
    default:
        break;
    }

    return RT_EOK;
}

int drv_xmac_init()
{
    for (int i = 0; i < sizeof(e2000q_xmac_obj) / sizeof(e2000q_xmac_obj[0]); i++)
    {
        if (e2000q_xmac_config_init(&e2000q_xmac_obj[i]) != RT_EOK)
        {
            PRT_Printf("xmac config %s err\n", e2000q_xmac_obj[i].device_name);
            return -RT_ERROR;
        }

        if (e2000q_xmac_gpio_init(&e2000q_xmac_obj[i]) != RT_EOK)
        {
            PRT_Printf("xmac gpio %s err\n", e2000q_xmac_obj[i].instance_id);
            return -RT_ERROR;
        }
        rt_device_t device = &e2000q_xmac_obj[i].device;

        memset(device, 0, sizeof(struct rt_device));

        device->type = RT_Device_Class_NetIf;
        device->init = e2000q_xmac_init;
        device->open = e2000q_xmac_open;
        device->close = e2000q_xmac_close;
        device->read = e2000q_xmac_read;
        device->write = e2000q_xmac_write;
        device->control = e2000q_xmac_control;
        device->user_data = &e2000q_xmac_obj[i];

        if (rt_device_register(device, e2000q_xmac_obj[i].device_name, RT_DEVICE_FLAG_RDWR) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
