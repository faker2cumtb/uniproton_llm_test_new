/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-10-14     LuoYuncong   the first version
 * 2024-11-05     LuoYuncong   fix 16 data bits bug
 */
#include <rtdevice.h>
#include <prt_hwi.h>
#include <fiopad.h>
#include "drv_common.h"
#include "drv_localbus.h"

static struct rt_device lbc_device;
static int lbc_pss_pin;

static rt_err_t lbc_init(rt_device_t dev)
{
    /* LBC_AD0 ~ LBC_AD10 */
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_N33_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_L33_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_N45_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_N43_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_L31_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_J31_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_J29_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_E29_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_G29_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_N27_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_L29_REG0_OFFSET, FIOPAD_FUNC0);

    /* LBC_ADDR23 ~ LBC_ADDR27 */
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_E35_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_G35_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_J35_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_L37_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_N35_REG0_OFFSET, FIOPAD_FUNC0);

    /* LBC_ALE, LBC_CS0, LBC_CS1, LBC_OE */
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_E31_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_G31_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_N41_REG0_OFFSET, FIOPAD_FUNC0);
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_N39_REG0_OFFSET, FIOPAD_FUNC0);

    /* LBC_WE */
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_AJ53_REG0_OFFSET, FIOPAD_FUNC5);

    /* default: 8bit */
    lbc_pss_pin = rt_pin_get("P0.1");
    rt_pin_mode(lbc_pss_pin, PIN_MODE_OUTPUT);
    rt_pin_write(lbc_pss_pin, PIN_HIGH);

    return RT_EOK;
}

static rt_err_t lbc_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t lbc_close(rt_device_t dev)
{
    return RT_ERROR;
}

static rt_size_t lbc_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return 0;
}

static rt_size_t lbc_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    return 0;
}

static rt_err_t lbc_control(rt_device_t dev, int cmd, void *arg)
{
    int bits = (int)((uintptr_t)arg);

    if ((bits != 8) && (bits != 16))
    {
        return RT_ERROR;
    }

    if (cmd != LBC_CTRL_DATA_BITS)
    {
        return RT_ERROR;
    }

    if (bits == 8) /* 8 data bits */
    {
        /* LBC_AD11 ~ LBC_AD15 */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J37_REG0_OFFSET, FIOPAD_FUNC1);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J39_REG0_OFFSET, FIOPAD_FUNC1);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_G41_REG0_OFFSET, FIOPAD_FUNC1);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_E43_REG0_OFFSET, FIOPAD_FUNC1);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_L43_REG0_OFFSET, FIOPAD_FUNC1);

        rt_pin_write(lbc_pss_pin, PIN_HIGH);

        *(uint32_t *)0x28007000 = 0x01;
        *(uint32_t *)0x28007010 = 0x80;
        *(uint32_t *)0x28007058 = 0x2d;
        *(uint32_t *)0x2800719c = 0x1201;
        *(uint32_t *)0x280071a0 = 0x404c;
        *(uint32_t *)0x280071a4 = 0x2106;
    }
    else /* 16 data bits */
    {
        /* LBC_AD11 ~ LBC_AD15 */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J37_REG0_OFFSET, FIOPAD_FUNC0);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J39_REG0_OFFSET, FIOPAD_FUNC0);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_G41_REG0_OFFSET, FIOPAD_FUNC0);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_E43_REG0_OFFSET, FIOPAD_FUNC0);
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_L43_REG0_OFFSET, FIOPAD_FUNC0);

        rt_pin_write(lbc_pss_pin, PIN_LOW);

        *(uint32_t *)0x28007000 = 0x01;
        *(uint32_t *)0x28007010 = 0x80;
        *(uint32_t *)0x28007058 = 0x25;
        *(uint32_t *)0x2800719c = 0x1201;
        *(uint32_t *)0x280071a0 = 0x404c;
        *(uint32_t *)0x280071a4 = 0x2106;
    }

    return RT_EOK;
}

int drv_localbus_init()
{
    memset(&lbc_device, 0, sizeof(struct rt_device));

    lbc_device.type = RT_Device_Class_MTD;
    lbc_device.init = lbc_init;
    lbc_device.open = lbc_open;
    lbc_device.close = lbc_close;
    lbc_device.read = lbc_read;
    lbc_device.write = lbc_write;
    lbc_device.control = lbc_control;

    if (rt_device_register(&lbc_device, "localbus", RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}
