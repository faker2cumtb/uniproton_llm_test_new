/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-20     ShiAnhu      the first version
 */
#include <errno.h>
#include "print.h"
#include "modbus/modbus-rtu.h"

extern int usleep(unsigned sleep_time_in_us);

int Modbus_Client_RTU_Demo()
{
#if !defined(OS_OPTION_PROXY)
    const char *ip_or_device = "uart2";
#else
    const char *ip_or_device = "mio_uart0";
#endif
    const int baud_rate = 115200;
    const char parity = 'N';
    const int data_bit = 8;
    const int stop_bit = 1;
    const bool debug_flag = false;
    const int sleep_time_us = debug_flag ? 1000 * 1000 : 1 * 1000;

    PRT_Printf("Hello world! - IDE: Visual Studio Code, OS: UniProton, "
               "Demo: Modbus Client RTU, Built: %s %s\n", __DATE__, __TIME__);
    PRT_Printf("=== UART: %s, baud: %d bps, parity: %c, data: %d, stop: %d ===\n",
               ip_or_device, baud_rate, parity, data_bit, stop_bit);

    modbus_t *ctx = modbus_new_rtu(ip_or_device, baud_rate, parity, data_bit, stop_bit);
    if (ctx == NULL)
    {
        PRT_Printf("ERROR: Unable to allocate libmodbus context\n");
        return -1;
    }

    if (modbus_set_debug(ctx, debug_flag) == -1)
    {
        PRT_Printf("ERROR: Set debug failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    if (modbus_set_slave(ctx, 1) == -1)
    {
        PRT_Printf("ERROR: Set slave failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    if (modbus_connect(ctx) == -1)
    {
        PRT_Printf("ERROR: Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    const int num_reg = 10;
    uint16_t tab_reg[64] = {0};
    int data_index = 0;

    while (1)
    {
        PRT_Printf("---------------- [MODBUS CLIENT RTU %03d] ----------------\n", ++data_index);

        int rc = modbus_read_registers(ctx, 0, num_reg, tab_reg);
        if (rc == -1)
        {
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }

        PRT_Printf("RECV REG[%d]<dec(hex)>: "
                   "%02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) "
                   "%02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X)\n",
                   num_reg,
                   tab_reg[0], tab_reg[0], tab_reg[1], tab_reg[1],
                   tab_reg[2], tab_reg[2], tab_reg[3], tab_reg[3],
                   tab_reg[4], tab_reg[4], tab_reg[5], tab_reg[5],
                   tab_reg[6], tab_reg[6], tab_reg[7], tab_reg[7],
                   tab_reg[8], tab_reg[8], tab_reg[9], tab_reg[9]);

        usleep(sleep_time_us);
    }

    PRT_Printf("ERROR: Quit the loop: %s\n", modbus_strerror(errno));

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
