/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     LuoYuncong   the first version
 */
#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

#include "prt_buildef.h"

#define RT_NAME_MAX 16
#define RT_ALIGN_SIZE 8

#define RT_USING_LIBC
#define RT_USING_DEVICE

/* GPIO, 对应引脚设置为true则使能，否则设置为0 */
#define RT_USING_PIN

#define BSP_USING_P0_A0 0
#define BSP_USING_P0_A1 0
#define BSP_USING_P0_A2 0
#define BSP_USING_P0_A3 0
#define BSP_USING_P0_A4 0
#define BSP_USING_P0_A5 0
#define BSP_USING_P0_A6 0
#define BSP_USING_P0_A7 0

#define BSP_USING_P0_B0 0
#define BSP_USING_P0_B1 0
#define BSP_USING_P0_B2 0
#define BSP_USING_P0_B3 0
#define BSP_USING_P0_B4 0
#define BSP_USING_P0_B5 0
#define BSP_USING_P0_B6 0
#define BSP_USING_P0_B7 0

#define BSP_USING_P0_C0 0
#define BSP_USING_P0_C1 0
#define BSP_USING_P0_C2 0
#define BSP_USING_P0_C3 0
#define BSP_USING_P0_C4 true
#define BSP_USING_P0_C5 true
#define BSP_USING_P0_C6 0
#define BSP_USING_P0_C7 0

#define BSP_USING_P0_D0 0
#define BSP_USING_P0_D1 0
#define BSP_USING_P0_D2 0
#define BSP_USING_P0_D3 true
#define BSP_USING_P0_D4 0
#define BSP_USING_P0_D5 0
#define BSP_USING_P0_D6 0
#define BSP_USING_P0_D7 0

#define BSP_USING_P1_A0 0
#define BSP_USING_P1_A1 0
#define BSP_USING_P1_A2 0
#define BSP_USING_P1_A3 0
#define BSP_USING_P1_A4 0
#define BSP_USING_P1_A5 0
#define BSP_USING_P1_A6 0
#define BSP_USING_P1_A7 0

#define BSP_USING_P1_B0 0
#define BSP_USING_P1_B1 0
#define BSP_USING_P1_B2 0
#define BSP_USING_P1_B3 0
#define BSP_USING_P1_B4 0
#define BSP_USING_P1_B5 0
#define BSP_USING_P1_B6 0
#define BSP_USING_P1_B7 0

#define BSP_USING_P1_C0 0
#define BSP_USING_P1_C1 0
#define BSP_USING_P1_C2 0
#define BSP_USING_P1_C3 0
#define BSP_USING_P1_C4 0
#define BSP_USING_P1_C5 0
#define BSP_USING_P1_C6 0
#define BSP_USING_P1_C7 0

#define BSP_USING_P1_D0 0
#define BSP_USING_P1_D1 0
#define BSP_USING_P1_D2 0
#define BSP_USING_P1_D3 0
#define BSP_USING_P1_D4 0
#define BSP_USING_P1_D5 0
#define BSP_USING_P1_D6 0
#define BSP_USING_P1_D7 0

#define BSP_USING_P2_A0 0
#define BSP_USING_P2_A1 0
#define BSP_USING_P2_A2 0
#define BSP_USING_P2_A3 0
#define BSP_USING_P2_A4 0
#define BSP_USING_P2_A5 0
#define BSP_USING_P2_A6 0
#define BSP_USING_P2_A7 0

#define BSP_USING_P2_B0 0
#define BSP_USING_P2_B1 0
#define BSP_USING_P2_B2 0
#define BSP_USING_P2_B3 0
#define BSP_USING_P2_B4 0
#define BSP_USING_P2_B5 0
#define BSP_USING_P2_B6 0
#define BSP_USING_P2_B7 0

#define BSP_USING_P2_C0 0
#define BSP_USING_P2_C1 0
#define BSP_USING_P2_C2 0
#define BSP_USING_P2_C3 0
#define BSP_USING_P2_C4 0
#define BSP_USING_P2_C5 0
#define BSP_USING_P2_C6 0
#define BSP_USING_P2_C7 0

#define BSP_USING_P2_D0 0
#define BSP_USING_P2_D1 0
#define BSP_USING_P2_D2 0
#define BSP_USING_P2_D3 0
#define BSP_USING_P2_D4 0
#define BSP_USING_P2_D5 0
#define BSP_USING_P2_D6 0
#define BSP_USING_P2_D7 0

#define BSP_USING_P3_A0 0
#define BSP_USING_P3_A1 true
#define BSP_USING_P3_A2 true
#define BSP_USING_P3_A3 true
#define BSP_USING_P3_A4 true
#define BSP_USING_P3_A5 true
#define BSP_USING_P3_A6 true
#define BSP_USING_P3_A7 true

#define BSP_USING_P3_B0 true
#define BSP_USING_P3_B1 0
#define BSP_USING_P3_B2 0
#define BSP_USING_P3_B3 true
#define BSP_USING_P3_B4 true
#define BSP_USING_P3_B5 true
#define BSP_USING_P3_B6 true
#define BSP_USING_P3_B7 0

#define BSP_USING_P3_C0 0
#define BSP_USING_P3_C1 true
#define BSP_USING_P3_C2 0
#define BSP_USING_P3_C3 0
#define BSP_USING_P3_C4 0
#define BSP_USING_P3_C5 0
#define BSP_USING_P3_C6 0
#define BSP_USING_P3_C7 0

#define BSP_USING_P3_D0 0
#define BSP_USING_P3_D1 0
#define BSP_USING_P3_D2 0
#define BSP_USING_P3_D3 0
#define BSP_USING_P3_D4 0
#define BSP_USING_P3_D5 0
#define BSP_USING_P3_D6 0
#define BSP_USING_P3_D7 0

#define BSP_USING_P4_A0 0
#define BSP_USING_P4_A1 0
#define BSP_USING_P4_A2 0
#define BSP_USING_P4_A3 0
#define BSP_USING_P4_A4 0
#define BSP_USING_P4_A5 0
#define BSP_USING_P4_A6 0
#define BSP_USING_P4_A7 0

#define BSP_USING_P4_B0 0
#define BSP_USING_P4_B1 0
#define BSP_USING_P4_B2 0
#define BSP_USING_P4_B3 0
#define BSP_USING_P4_B4 0
#define BSP_USING_P4_B5 0
#define BSP_USING_P4_B6 0
#define BSP_USING_P4_B7 0

#define BSP_USING_P4_C0 0
#define BSP_USING_P4_C1 0
#define BSP_USING_P4_C2 0
#define BSP_USING_P4_C3 0
#define BSP_USING_P4_C4 0
#define BSP_USING_P4_C5 0
#define BSP_USING_P4_C6 0
#define BSP_USING_P4_C7 0

#define BSP_USING_P4_D0 0
#define BSP_USING_P4_D1 0
#define BSP_USING_P4_D2 0
#define BSP_USING_P4_D3 0
#define BSP_USING_P4_D4 0
#define BSP_USING_P4_D5 0
#define BSP_USING_P4_D6 0
#define BSP_USING_P4_D7 0

/* UART */
#define RT_USING_SERIAL
#define BSP_USING_UART3
#define BSP_USING_UART3_M1
#define BSP_UART3_BAUDRATE 115200

/* I2C */
#define RT_USING_I2C
// #define BSP_USING_I2C0
// #define BSP_USING_I2C0_M1
// #define BSP_I2C0_BAUDRATE 100000

/* SPI */
#define RT_USING_SPI
// #define BSP_USING_SPI0
// #define BSP_USING_SPI0_M1
// #define BSP_USING_SPI0_M1_CS_INDEX 0
// #define BSP_SPI0_BAUDRATE 10000000

/* CAN */
#define RT_USING_CAN
#define RT_CAN_USING_HDR
// #define BSP_USING_CAN1
// #define BSP_USING_CAN1_M1
// #define BSP_CAN1_BAUDRATE 1000000

/* Hardware Timer */
#define RT_USING_HWTIMER
#define BSP_USING_TIMER0
#define BSP_USING_TIMER1
#define BSP_USING_TIMER2

#endif
