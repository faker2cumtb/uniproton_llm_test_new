/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-10-14     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <drv_localbus.h>
#include <demo.h>

void localbus_demo()
{
    rt_device_t localbus_dev = rt_device_find("localbus");

    if (localbus_dev == RT_NULL)
    {
        PRT_Printf("localbus device not found\n");
        return;
    }

    rt_device_open(localbus_dev, RT_DEVICE_FLAG_RDWR);

    for (int data_bits = 8; data_bits <= 16; data_bits += 8)
    {
        uint16_t *p_addr;
        uint16_t val = 0x1234;

        PRT_Printf("================================\n");
        PRT_Printf("Set localbus data bits: %d\n\n", (void *)data_bits);

        rt_device_control(localbus_dev, LBC_CTRL_DATA_BITS, data_bits);

        /* FPGA Version */
        PRT_Printf("FPGA VERSION: %#x\n", *(uint16_t *)0x11000208);

        /* Hardware Version */
        PRT_Printf("HW VERSION: %#x\n", *(uint16_t *)0x1100020a);

        /* GSK Link Version */
        if (data_bits == 16)
        {
            PRT_Printf("GLINK VERSION: %#x\n", *(uint16_t *)0x10000226);
            val = 0x5678;
        }

        PRT_Printf("\n");

        /* Reg test */
        p_addr = (uint16_t *)0x11000202;
        *p_addr = val;
        PRT_Printf("Reg [%#x], write %#x, read %#x, %s\n", p_addr, val, *p_addr, *p_addr == val ? "success" : "failed");
        PRT_Printf("\n");

        /* RAM test */
        for (p_addr = (uint16_t *)0x11800000; p_addr < (uint16_t *)0x11800800; p_addr += 32)
        {
            *p_addr = val;
            PRT_Printf("RAM [%#x], write %#x, read %#x, %s\n", p_addr, val, *p_addr, *p_addr == val ? "success" : "failed");
        }
        PRT_Printf("\n");

        /* GSK link Test */
        if (data_bits == 16)
        {
            *(uint16_t *)0x10000256 = 0x8000;
            for (p_addr = (uint16_t *)0x10000100; p_addr < (uint16_t *)0x10000200; p_addr += 16)
            {
                *p_addr = val;
                PRT_Printf("GLINK [%#x], write %#x, read %#x, %s\n", p_addr, val, *p_addr, *p_addr == val ? "success" : "failed");
            }
            PRT_Printf("\n");
        }
    }
}
