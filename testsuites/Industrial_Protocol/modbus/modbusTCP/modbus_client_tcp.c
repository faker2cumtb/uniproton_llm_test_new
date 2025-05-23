/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-24     huajian      the first version
 */
#include <errno.h>
#include "print.h"
#include "lwip/etharp.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include "modbus/modbus-tcp.h"
#include "modbus_port.h"

void Net_Lwip_Init(void);

#define MICA_TEST
modbus_t *ctx;
const bool debug_flag = false;  
const int sleep_time_us = debug_flag ? 1000 * 1000 : 1 * 1000; 
int data_index = 0;

void check(int rc, const char *msg) {
    if (rc == -1) {
        fprintf(stderr, "Error: %s\n", modbus_strerror(errno));
        exit(1);
    }
}

void modbus_tcp_client_init()
{
    const char *ip_or_device = "192.168.66.248";  
    const int srv_port = 502; 

    PRT_Printf("Hello world! - IDE: Visual Studio Code, OS: UniProton, "  
               "Demo: Modbus Client TCP, Built: %s %s\n", __DATE__, __TIME__);  
    PRT_Printf("Server IP: %s, Server Port: %d\n", ip_or_device, srv_port); 

#if defined(MAC_TEST)
    Net_Lwip_Init(); // init LWIP and MAC 
#elif defined(MICA_TEST)
    mica_service_init();
#endif 

    ctx = modbus_new_tcp(ip_or_device, srv_port);  
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

    if (modbus_connect(ctx) == -1)  
    {  
        PRT_Printf("ERROR: Connection failed: %s\n", modbus_strerror(errno));  
        modbus_free(ctx);  
        return -1;  
    }  
}

int Modbus_Client_TCP()  
{  
    modbus_tcp_client_init();

    while (1)  
    {  
        // 打印循环的开始  
        PRT_Printf("---------------- [MODBUS CLIENT TCP %03d] ----------------\n", ++data_index);  
        // 功能码 0x01 - 读取线圈状态
        uint8_t tab_rp_bits[64];
        int rc = modbus_read_bits(ctx, 0, 64, tab_rp_bits); // 读取起始地址为0的64个线圈状态
        check(rc, "Failed to read coils");

        // 功能码 0x02 - 读取离散输入状态
        uint8_t tab_di_bits[64];
        rc = modbus_read_input_bits(ctx, 0, 64, tab_di_bits); // 读取起始地址为0的64个离散输入状态
        check(rc, "Failed to read discrete inputs");

        // 功能码 0x03 - 读取保持寄存器
        uint16_t tab_rp_registers[64];
        rc = modbus_read_registers(ctx, 0, 64, tab_rp_registers); // 读取起始地址为0的64个保持寄存器
        check(rc, "Failed to read holding registers");

        // 功能码 0x04 - 读取输入寄存器
        uint16_t tab_ir_registers[64];
        rc = modbus_read_input_registers(ctx, 0, 64, tab_ir_registers); // 读取起始地址为0的64个输入寄存器
        check(rc, "Failed to read input registers");

        // 功能码 0x05 - 写单个线圈
        rc = modbus_write_bit(ctx, 0, ON); // 写入ON状态到起始地址为0的单个线圈
        check(rc, "Failed to write single coil");

        // 功能码 0x06 - 写单个保持寄存器
        rc = modbus_write_register(ctx, 0, 0xABCD); // 写入值0xABCD到起始地址为0的单个保持寄存器
        check(rc, "Failed to write single register");

        // 功能码 0x0F - 写多个线圈
        uint8_t tab_w_bits[64] = {0};
        memset(tab_w_bits, 0xFF, 64); // 设置所有线圈为ON状态
        rc = modbus_write_bits(ctx, 0, 64, tab_w_bits); // 写入64个线圈的状态
        check(rc, "Failed to write multiple coils");

        // 功能码 0x10 - 写多个保持寄存器
        uint16_t tab_w_registers[64] = {0};
        for (int i = 0; i < 64; i++) {
            tab_w_registers[i] = i + 1;
        }
        rc = modbus_write_registers(ctx, 0, 64, tab_w_registers); // 写入64个保持寄存器的值
        check(rc, "Failed to write multiple registers");

        usleep(sleep_time_us);  
    }  

    PRT_Printf("ERROR: Quit the loop: %s\n", modbus_strerror(errno));  

    modbus_close(ctx);  
    modbus_free(ctx);  

    return 0;  
}