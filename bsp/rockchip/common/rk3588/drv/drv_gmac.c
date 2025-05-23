/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-13     LuoYuncong   the first version
 * 2025-01-06     LuoYuncong   fix bugs
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <hwi_router.h>
#include <hal_base.h>
#include <rk_gmac.h>
#include "drv_gmac.h"

/* 当前固定为16个发送描述符，16个接收描述符 */
extern U64 g_gmac_tx_rx_desc_begin;

struct rk3588_gmac
{
    char *device_name;

    HwiHandle intr_num;
    HwiPrior intr_prio;

    int bus_id;
    int phy_addr;

    /* phy chip reset pin */
    eGPIO_bankId phy_reset_bank;
    struct GPIO_REG *phy_reset_gpio;
    uint32_t phy_reset_pin;
    ePINCTRL_configParam phy_reset_param;

    /* mdio interface */
    eGPIO_bankId phy_bank;
    uint32_t phy_mdc_pin;
    uint32_t phy_mdio_pin;
    ePINCTRL_configParam phy_param;

    /* gmac rgmii interface */
    eGPIO_bankId gmac_comm_bank;
    eGPIO_bankId gmac_rxdv_bank;
    uint32_t gmac_rxclk_pin;
    uint32_t gmac_rxdv_pin;
    uint32_t gmac_rxd0_pin;
    uint32_t gmac_rxd1_pin;
    uint32_t gmac_rxd2_pin;
    uint32_t gmac_rxd3_pin;
    uint32_t gmac_txclk_pin;
    uint32_t gmac_txen_pin;
    uint32_t gmac_txd0_pin;
    uint32_t gmac_txd1_pin;
    uint32_t gmac_txd2_pin;
    uint32_t gmac_txd3_pin;
    ePINCTRL_configParam gmac_comm_param;
    ePINCTRL_configParam gmac_rxdv_param;

    struct rk_gmac_dev gmac_dev;
    struct rt_device device;
};

static struct rk3588_gmac gmac_obj[] = {
#if defined(BSP_USING_GMAC0)
    {
        .device_name = "gmac0",

        .intr_num = 259,
        .intr_prio = 4,

        .bus_id = 0,
        .phy_addr = 0,

        .phy_reset_bank = GPIO_BANK3,
        .phy_reset_gpio = GPIO3,
        .phy_reset_pin = GPIO_PIN_B2,
        .phy_reset_param = PIN_CONFIG_PUL_UP,

        .phy_bank = GPIO_BANK4,
        .phy_mdc_pin = GPIO_PIN_C4,
        .phy_mdio_pin = GPIO_PIN_C5,
        .phy_param = PIN_CONFIG_MUX_FUNC1,

        .gmac_comm_bank = GPIO_BANK2,
        .gmac_rxdv_bank = GPIO_BANK4,

        .gmac_rxclk_pin = GPIO_PIN_B0,
        .gmac_rxdv_pin = GPIO_PIN_C2,
        .gmac_rxd0_pin = GPIO_PIN_C1,
        .gmac_rxd1_pin = GPIO_PIN_C2,
        .gmac_rxd2_pin = GPIO_PIN_A6,
        .gmac_rxd3_pin = GPIO_PIN_A7,

        .gmac_txclk_pin = GPIO_PIN_B3,
        .gmac_txen_pin = GPIO_PIN_C0,
        .gmac_txd0_pin = GPIO_PIN_B6,
        .gmac_txd1_pin = GPIO_PIN_B7,
        .gmac_txd2_pin = GPIO_PIN_B1,
        .gmac_txd3_pin = GPIO_PIN_B2,

        .gmac_comm_param = PIN_CONFIG_MUX_FUNC1,
        .gmac_rxdv_param = PIN_CONFIG_MUX_FUNC1,
    },
#endif

#if defined(BSP_USING_GMAC1)
    {
        .device_name = "gmac1",

        .intr_num = 266,
        .intr_prio = 4,

        .bus_id = 1,
        .phy_addr = 1,

        .phy_reset_bank = GPIO_BANK3,
        .phy_reset_gpio = GPIO3,
        .phy_reset_pin = GPIO_PIN_B6,
        .phy_reset_param = PIN_CONFIG_PUL_UP,

        .phy_bank = GPIO_BANK3,
        .phy_mdc_pin = GPIO_PIN_C2,
        .phy_mdio_pin = GPIO_PIN_C3,
        .phy_param = PIN_CONFIG_MUX_FUNC1,

        .gmac_comm_bank = GPIO_BANK3,
        .gmac_rxdv_bank = GPIO_BANK3,

        .gmac_rxclk_pin = GPIO_PIN_A5,
        .gmac_rxdv_pin = GPIO_PIN_B1,
        .gmac_rxd0_pin = GPIO_PIN_A7,
        .gmac_rxd1_pin = GPIO_PIN_B0,
        .gmac_rxd2_pin = GPIO_PIN_A2,
        .gmac_rxd3_pin = GPIO_PIN_A3,

        .gmac_txclk_pin = GPIO_PIN_A4,
        .gmac_txen_pin = GPIO_PIN_B5,
        .gmac_txd0_pin = GPIO_PIN_B3,
        .gmac_txd1_pin = GPIO_PIN_B4,
        .gmac_txd2_pin = GPIO_PIN_A0,
        .gmac_txd3_pin = GPIO_PIN_A1,

        .gmac_comm_param = PIN_CONFIG_MUX_FUNC1,
        .gmac_rxdv_param = PIN_CONFIG_MUX_FUNC1,
    },
#endif

};

static void rk3588_gmac_isr(uintptr_t arg)
{
    struct rk3588_gmac *drv_gmac = (struct rk3588_gmac *)arg;

    uint32_t status = rk_gmac_intr_status(&drv_gmac->gmac_dev);

    if ((status & EQOS_DMA_CH0_INTR_ENA_RIE) || (status & EQOS_DMA_CH0_INTR_ENA_ERIE))
    {
        if (drv_gmac->device.rx_indicate != RT_NULL)
        {
            drv_gmac->device.rx_indicate(&drv_gmac->device, RT_NULL);
        }
    }

    if ((status & EQOS_DMA_CH0_INTR_ENA_TIE) || (status & EQOS_DMA_CH0_INTR_ENA_ETIE))
    {
        if (drv_gmac->device.tx_complete != RT_NULL)
        {
            drv_gmac->device.tx_complete(&drv_gmac->device, RT_NULL);
        }
    }
}

rt_err_t rk3588_gmac_phy_init(struct rk3588_gmac *drv_gmac)
{
    if (HAL_PINCTRL_SetIOMUX(drv_gmac->phy_reset_bank, drv_gmac->phy_reset_pin, drv_gmac->phy_reset_param) != HAL_OK)
    {
        return -RT_ERROR;
    }

    HAL_GPIO_SetPinDirection(drv_gmac->phy_reset_gpio, drv_gmac->phy_reset_pin, GPIO_OUT);

    return RT_EOK;
}

rt_err_t rk3588_gmac_gpio_init(struct rk3588_gmac *drv_gmac)
{
    HAL_Status status;

    status = HAL_PINCTRL_SetIOMUX(drv_gmac->phy_bank, drv_gmac->phy_mdc_pin | drv_gmac->phy_mdio_pin, drv_gmac->phy_param);
    if (status != HAL_OK)
    {
        return -RT_ERROR;
    }

    status = HAL_PINCTRL_SetIOMUX(drv_gmac->gmac_rxdv_bank, drv_gmac->gmac_rxdv_pin, drv_gmac->gmac_rxdv_param);
    if (status != HAL_OK)
    {
        return -RT_ERROR;
    }

    status = HAL_PINCTRL_SetIOMUX(drv_gmac->gmac_comm_bank,
                                  drv_gmac->gmac_rxclk_pin | drv_gmac->gmac_rxd0_pin | drv_gmac->gmac_rxd1_pin |
                                      drv_gmac->gmac_rxd2_pin | drv_gmac->gmac_rxd3_pin | drv_gmac->gmac_txclk_pin |
                                      drv_gmac->gmac_txen_pin | drv_gmac->gmac_txd0_pin | drv_gmac->gmac_txd1_pin |
                                      drv_gmac->gmac_txd2_pin | drv_gmac->gmac_txd3_pin,
                                  drv_gmac->gmac_comm_param);
    if (status != HAL_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rk3588_gmac_config_init(struct rk3588_gmac *drv_gmac)
{
    struct eqos_priv *eqos = &drv_gmac->gmac_dev.eqos;
    struct rk_gmac_platdata *pdata = &drv_gmac->gmac_dev.platdata;
    uintptr_t desc_begin = (uintptr_t)&g_gmac_tx_rx_desc_begin;

    memset(&drv_gmac->gmac_dev, 0, sizeof(drv_gmac->gmac_dev));

    eqos->phy_addr = drv_gmac->phy_addr;
    eqos->max_speed = SPEED_1000;
    eqos->descs = (void *)(desc_begin + drv_gmac->bus_id * 16 * 1024);
    eqos->interface = PHY_INTERFACE_MODE_RGMII_RXID;

    pdata->clock_input = false;
    pdata->tx_delay = 0x30;
    pdata->rx_delay = 0x10;
    pdata->bus_id = drv_gmac->bus_id;

    return RT_EOK;
}

rt_err_t rk3588_gmac_intr_init(struct rk3588_gmac *drv_gmac)
{
    if (PRT_HwiSetAttr(drv_gmac->intr_num, drv_gmac->intr_prio, OS_HWI_MODE_ENGROSS) != OS_OK)
    {
        return -RT_ERROR;
    }

    if (PRT_HwiCreate(drv_gmac->intr_num, (HwiProcFunc)rk3588_gmac_isr, (uintptr_t)drv_gmac) != OS_OK)
    {
        return -RT_ERROR;
    }

    PRT_HwiSetRouter(drv_gmac->intr_num);

    return RT_EOK;
}

rt_err_t rk3588_gmac_init(rt_device_t dev)
{
    struct rk3588_gmac *drv_gmac = (struct rk3588_gmac *)dev->user_data;

    HAL_GPIO_SetPinLevel(drv_gmac->phy_reset_gpio, drv_gmac->phy_reset_pin, GPIO_LOW);
    HAL_DelayMs(100);
    HAL_GPIO_SetPinLevel(drv_gmac->phy_reset_gpio, drv_gmac->phy_reset_pin, GPIO_HIGH);
    HAL_DelayMs(1);

    return RT_EOK;
}

rt_err_t rk3588_gmac_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct rk3588_gmac *drv_gmac = (struct rk3588_gmac *)dev->user_data;

    if (rk_gmac_probe(&drv_gmac->gmac_dev) != 0)
    {
        return -RT_ERROR;
    }

    if (rk_gmac_start(&drv_gmac->gmac_dev) != 0)
    {
        return -RT_ERROR;
    }

    if (PRT_HwiEnable(drv_gmac->intr_num) != OS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rk3588_gmac_close(rt_device_t dev)
{
    struct rk3588_gmac *drv_gmac = (struct rk3588_gmac *)dev->user_data;

    PRT_HwiDisable(drv_gmac->intr_num);
    rk_gmac_stop(&drv_gmac->gmac_dev);

    return RT_EOK;
}

rt_size_t rk3588_gmac_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct rk3588_gmac *drv_gmac = (struct rk3588_gmac *)dev->user_data;
    uint8_t *rx_buf;
    int length;

    if ((length = rk_gmac_recv(&drv_gmac->gmac_dev, 0, &rx_buf)) > 0)
    {
        int len = (length < size) ? length : size;

        memcpy(buffer, rx_buf, len);
        rk_gmac_free_pkt(&drv_gmac->gmac_dev, rx_buf, length);

        return len;
    }

    return 0;
}

rt_size_t rk3588_gmac_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct rk3588_gmac *drv_gmac = (struct rk3588_gmac *)dev->user_data;
    uint8_t *tx_buf = (uint8_t *)buffer;

    if (rk_gmac_send(&drv_gmac->gmac_dev, tx_buf, size) == 0)
    {
        return size;
    }

    return 0;
}

rt_err_t rk3588_gmac_control(rt_device_t dev, int cmd, void *arg)
{
    struct rk3588_gmac *drv_gmac = (struct rk3588_gmac *)dev->user_data;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        PRT_HwiDisable(drv_gmac->intr_num);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        PRT_HwiEnable(drv_gmac->intr_num);
        break;
    default:
        break;
    }

    return RT_EOK;
}

int drv_gmac_init()
{
    for (int i = 0; i < sizeof(gmac_obj) / sizeof(gmac_obj[0]); i++)
    {
        if (rk3588_gmac_phy_init(&gmac_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_gmac_gpio_init(&gmac_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_gmac_config_init(&gmac_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        if (rk3588_gmac_intr_init(&gmac_obj[i]) != RT_EOK)
        {
            return -RT_ERROR;
        }

        rt_device_t device = &gmac_obj[i].device;

        memset(device, 0, sizeof(struct rt_device));

        device->type = RT_Device_Class_NetIf;
        device->init = rk3588_gmac_init;
        device->open = rk3588_gmac_open;
        device->close = rk3588_gmac_close;
        device->read = rk3588_gmac_read;
        device->write = rk3588_gmac_write;
        device->control = rk3588_gmac_control;
        device->user_data = &gmac_obj[i];

        if (rt_device_register(device, gmac_obj[i].device_name, RT_DEVICE_FLAG_RDWR) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
