/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-20     huajian      the first version
 */
#include <errno.h>
#include "print.h"
#include "modbus/modbus-rtu.h"
#include "modbus_port.h"

modbus_t *ctx;
const bool debug_flag = false;
const int sleep_time_us = debug_flag ? 1000 * 1000 : 1 * 1000;
uint16_t tab_reg[64] = {0};

void modbus_client_init(){
    const char *ip_or_device = "mio_uart14";
    const int baud_rate = 115200;
    const char parity = 'N';
    const int data_bit = 8;
    const int stop_bit = 1;

    PRT_Printf("Hello world! - IDE: Visual Studio Code, OS: UniProton, "
               "Demo: Modbus Client RTU, Built: %s %s\n", __DATE__, __TIME__);
    PRT_Printf("=== UART: %s, baud: %d bps, parity: %c, data: %d, stop: %d ===\n",
               ip_or_device, baud_rate, parity, data_bit, stop_bit);

    ctx = modbus_new_rtu(ip_or_device, baud_rate, 'N', 8, 1);
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
}

void print_val(){
    PRT_Printf("RECV <dec(hex)>: "
                   "%02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) "
                   "%02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X)\n",
                   tab_reg[0], tab_reg[0], tab_reg[1], tab_reg[1],
                   tab_reg[2], tab_reg[2], tab_reg[3], tab_reg[3],
                   tab_reg[4], tab_reg[4], tab_reg[5], tab_reg[5],
                   tab_reg[6], tab_reg[6], tab_reg[7], tab_reg[7],
                   tab_reg[8], tab_reg[8], tab_reg[9], tab_reg[9]);
}

int modbus_main()
{
    modbus_client_init();

    const int num_reg = 10;
    int data_index = 0;

    while (1)
    {
        PRT_Printf("---------------- [MODBUS CLIENT RTU %03d] ----------------\n", ++data_index);

        // 功能码 0x01 - 读取线圈状态
        uint8_t tab_rp_bits[64];
        int rc = modbus_read_bits(ctx, 0, num_reg, tab_rp_bits);
        if(rc == -1){
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }
      
        // 功能码 0x02 - 读取离散输入状态
        uint8_t tab_di_bits[64];
        rc = modbus_read_input_bits(ctx, 0, num_reg, tab_di_bits); 
        if (rc == -1)
        {
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }

        // 功能码 0x03 - 读取保持寄存器
        rc = modbus_read_registers(ctx, 0, num_reg, tab_reg);
        if (rc == -1)
        {
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }
        print_val();

        // 功能码 0x04 - 读取输入寄存器
        rc = modbus_read_input_registers(ctx, 0, num_reg, tab_reg); 
        if (rc == -1)
        {
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }
        print_val();

        // 功能码 0x05 - 写单个线圈
        rc = modbus_write_bit(ctx, 0, ON); 
        if (rc == -1)
        {
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }

        // 功能码 0x06 - 写单个保持寄存器
        rc = modbus_write_register(ctx, 0, 0xABCD); 
        if (rc == -1)
        {
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }

        // 功能码 0x0F - 写多个线圈
        uint8_t tab_w_bits[64] = {0};
        memset(tab_w_bits, 0xFF, 64); // 设置所有线圈为ON状态
        rc = modbus_write_bits(ctx, 0, num_reg, tab_w_bits); // 写入个线圈的状态
        if (rc == -1)
        {
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }

        // 功能码 0x10 - 写多个保持寄存器
        for (int i = 0; i < num_reg; i++) {
            tab_reg[i] = i + 1;
        }
        rc = modbus_write_registers(ctx, 0, num_reg, tab_reg); // 写入64个保持寄存器的值
        if (rc == -1)
        {
            PRT_Printf("ERROR: %s\n", modbus_strerror(errno));
            return -1;
        }

        usleep(sleep_time_us);
    }

    PRT_Printf("ERROR: Quit the loop: %s\n", modbus_strerror(errno));

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
