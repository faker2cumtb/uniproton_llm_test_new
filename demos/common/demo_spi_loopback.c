/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-30     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <demo.h>

void spi_loopback_demo()
{
    struct rt_spi_device spi_dev;
    struct rt_spi_configuration cfg;
    rt_uint16_t tx_data = 0x1000;
    rt_uint16_t rx_data = 0;

    /* 将spi_dev设备挂接到总SPI线，并重新命名 */
    if (rt_spi_bus_attach_device(&spi_dev, DEMO_SPI_LOOPBACK_DEV, DEMO_SPI_LOOPBACK_BUS, (void *)RT_NULL) != RT_EOK)
    {
        PRT_Printf("find device %s failed\n", DEMO_SPI_LOOPBACK_DEV);
        return;
    }

    PRT_Printf("find device %s success\n", DEMO_SPI_LOOPBACK_DEV);

    cfg.data_width = DEMO_SPI_LOOPBACK_DATA_WIDTH;
    cfg.mode = DEMO_SPI_LOOPBACK_MODE;
    cfg.max_hz = DEMO_SPI_LOOPBACK_BAUD;
    rt_spi_configure(&spi_dev, &cfg);

    while (1)
    {
        tx_data++;
        rt_spi_send_then_recv(&spi_dev, &tx_data, 2, &rx_data, 2);

        PRT_Printf("[SPI Test] tx: 0x%08x,  rx: 0x%08x [ %s ]\n", tx_data, rx_data, tx_data == rx_data ? "success" : "failed");

        PRT_TaskDelay(1000);
    }
}
