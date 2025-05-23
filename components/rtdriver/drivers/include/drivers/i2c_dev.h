/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2012-04-25     weety         first version
 * 2021-04-20     RiceChen      added bus clock command
 * 2024-07-19     LuoYuncong    Port to UniProton
 */

#ifndef __I2C_DEV_H__
#define __I2C_DEV_H__

#include <rtapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RT_I2C_DEV_CTRL_10BIT        (RT_DEVICE_CTRL_BASE(I2CBUS) + 0x01)
#define RT_I2C_DEV_CTRL_ADDR         (RT_DEVICE_CTRL_BASE(I2CBUS) + 0x02)
#define RT_I2C_DEV_CTRL_TIMEOUT      (RT_DEVICE_CTRL_BASE(I2CBUS) + 0x03)
#define RT_I2C_DEV_CTRL_RW           (RT_DEVICE_CTRL_BASE(I2CBUS) + 0x04)
#define RT_I2C_DEV_CTRL_CLK          (RT_DEVICE_CTRL_BASE(I2CBUS) + 0x05)

struct rt_i2c_priv_data
{
    struct rt_i2c_msg  *msgs;
    rt_size_t  number;
};

rt_err_t rt_i2c_bus_device_device_init(struct rt_i2c_bus_device *bus,
                                       const char               *name);

#ifdef __cplusplus
}
#endif

#endif
