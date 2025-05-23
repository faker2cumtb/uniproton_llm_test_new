/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-19     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include "drv_gpio.h"
#include "drv_uart.h"
#include "drv_i2c.h"
#include "drv_spi.h"
#include "drv_can.h"
#include "drv_timer.h"
#include "drv_xmac.h"
#include "drv_localbus.h"
#include "drv_common.h"
#include "print.h"

FIOPadCtrl iopad_ctrl;

int drv_device_init()
{
    FIOPadCfgInitialize(&iopad_ctrl, FIOPadLookupConfig(FIOPAD0_ID));

#if defined(RT_USING_PIN)
    if (drv_pin_init() != RT_EOK)
    {
        return -RT_ERROR;
    }
#endif

#if defined(RT_USING_SERIAL)
    if (drv_uart_init() != RT_EOK)
    {
        return -RT_ERROR;
    }
#endif

#if defined(RT_USING_I2C)
    if (drv_i2c_init() != RT_EOK)
    {
        return -RT_ERROR;
    }
#endif

#if defined(RT_USING_SPI)
    if (drv_spi_init() != RT_EOK)
    {
        return -RT_ERROR;
    }
#endif

#if defined(RT_USING_CAN)
    if (drv_can_init() != RT_EOK)
    {
        return -RT_ERROR;
    }
#endif

#if defined(RT_USING_HWTIMER)
    if (drv_timer_init() != RT_EOK)
    {
        return -RT_ERROR;
    }
#endif

#if defined(RT_USING_XMAC)
    if (drv_xmac_init() != RT_EOK)
    {
        return -RT_ERROR;
    }
#endif

#if defined(RT_USING_LOCALBUS)
    if (drv_localbus_init() != RT_EOK)
    {
        return -RT_ERROR;
    }
#endif

    return RT_EOK;
}
