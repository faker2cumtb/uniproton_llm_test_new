/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-05     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include "demo.h"

static struct rt_spi_device xl2515_dev;

static int xl2515_device_init()
{
    struct rt_spi_configuration cfg;

    /* 复位XL2515芯片 */
    rt_base_t reset_pin = rt_pin_get(DEMO_SPI_XL2515_CS_PIN);
    rt_pin_mode(reset_pin, PIN_MODE_OUTPUT);
    rt_pin_write(reset_pin, PIN_LOW);
    PRT_ClkDelayMs(10);
    rt_pin_write(reset_pin, PIN_HIGH);

    /* 将SPI_XL2515_DEV设备挂接SPI_XL2515_BUS总线上 */
    if (rt_spi_bus_attach_device(&xl2515_dev, DEMO_SPI_XL2515_DEV, DEMO_SPI_XL2515_BUS, (void *)RT_NULL) != RT_EOK)
    {
        PRT_Printf("rt_spi_bus_attach_device failed\n");
        return -RT_ERROR;
    }

    /* 配置SPI参数 */
    cfg.data_width = DEMO_SPI_XL2515_DATA_WIDTH;
    cfg.mode = DEMO_SPI_XL2515_MODE;
    cfg.max_hz = DEMO_SPI_XL2515_BAUD;
    rt_spi_configure(&xl2515_dev, &cfg);

    return RT_EOK;
}

void xl2515_write_byte(struct rt_spi_device *dev, uint8_t reg, uint8_t data)
{
    rt_uint8_t tx_buf[3];

    tx_buf[0] = 0x02;
    tx_buf[1] = reg;
    tx_buf[2] = data;

    rt_spi_send(dev, tx_buf, 3);
}

void xl2515_read_byte(struct rt_spi_device *dev, uint8_t reg, uint8_t *data)
{
    rt_uint8_t tx_buf[2];

    tx_buf[0] = 0x03;
    tx_buf[1] = reg;

    rt_spi_send_then_recv(dev, tx_buf, 2, data, 1);
}

void spi_xl2515_demo()
{
    xl2515_device_init();

    struct rt_spi_device *dev = (struct rt_spi_device *)rt_device_find(DEMO_SPI_XL2515_DEV);
    if (dev == RT_NULL)
    {
        PRT_Printf("find device %s failed\n", DEMO_SPI_XL2515_DEV);
    }

    PRT_Printf("find device %s success\n", DEMO_SPI_XL2515_DEV);

    /* 读写XL2515芯片0x28寄存器，此寄存器只能写入数值0-7 */
    for (int val = 0; val < 8; val++)
    {
        rt_uint8_t data;

        xl2515_write_byte(dev, 0x28, val);
        xl2515_read_byte(dev, 0x28, &data);

        if (data == val)
        {
            PRT_Printf("XL2515 REG[0x28] write %d, read %d. success\n", val, data);
        }
        else
        {
            PRT_Printf("XL2515 REG[0x28] write %d, read %d. failed\n", val, data);
        }

        PRT_ClkDelayMs(10);
    }
}
