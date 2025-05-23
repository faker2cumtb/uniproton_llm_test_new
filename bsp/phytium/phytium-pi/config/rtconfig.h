/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-19     LuoYuncong   the first version
 */
#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

#include "prt_buildef.h"

#define RT_NAME_MAX 16
#define RT_ALIGN_SIZE 8

#define RT_USING_LIBC
#define RT_USING_DEVICE

/* GPIO, 对应引脚设置为true为使能，不用的引脚必须设置为0 */
#define RT_USING_PIN

#define BSP_USING_P0_0 0
#define BSP_USING_P0_1 true
#define BSP_USING_P0_2 0
#define BSP_USING_P0_3 0
#define BSP_USING_P0_4 0
#define BSP_USING_P0_5 0
#define BSP_USING_P0_6 0
#define BSP_USING_P0_7 0
#define BSP_USING_P0_8 0
#define BSP_USING_P0_9 0
#define BSP_USING_P0_10 0
#define BSP_USING_P0_11 0
#define BSP_USING_P0_12 0
#define BSP_USING_P0_13 0
#define BSP_USING_P0_14 0
#define BSP_USING_P0_15 0

#define BSP_USING_P1_0 0
#define BSP_USING_P1_1 0
#define BSP_USING_P1_2 0
#define BSP_USING_P1_3 0
#define BSP_USING_P1_4 0
#define BSP_USING_P1_5 0
#define BSP_USING_P1_6 0
#define BSP_USING_P1_7 0
#define BSP_USING_P1_8 0
#define BSP_USING_P1_9 0
#define BSP_USING_P1_10 0
#define BSP_USING_P1_11 true
#define BSP_USING_P1_12 true
#define BSP_USING_P1_13 0
#define BSP_USING_P1_14 0
#define BSP_USING_P1_15 true

#define BSP_USING_P2_0 0
#define BSP_USING_P2_1 0
#define BSP_USING_P2_2 true
#define BSP_USING_P2_3 0
#define BSP_USING_P2_4 0
#define BSP_USING_P2_5 0
#define BSP_USING_P2_6 0
#define BSP_USING_P2_7 0
#define BSP_USING_P2_8 true
#define BSP_USING_P2_9 0
#define BSP_USING_P2_10 0
#define BSP_USING_P2_11 0
#define BSP_USING_P2_12 0
#define BSP_USING_P2_13 0
#define BSP_USING_P2_14 0
#define BSP_USING_P2_15 0

#define BSP_USING_P3_0 0
#define BSP_USING_P3_1 0
#define BSP_USING_P3_2 0
#define BSP_USING_P3_3 0
#define BSP_USING_P3_4 0
#define BSP_USING_P3_5 0
#define BSP_USING_P3_6 0
#define BSP_USING_P3_7 0
#define BSP_USING_P3_8 0
#define BSP_USING_P3_9 0
#define BSP_USING_P3_10 0
#define BSP_USING_P3_11 0
#define BSP_USING_P3_12 0
#define BSP_USING_P3_13 0
#define BSP_USING_P3_14 0
#define BSP_USING_P3_15 0

#define BSP_USING_P4_0 0
#define BSP_USING_P4_1 0
#define BSP_USING_P4_2 0
#define BSP_USING_P4_3 0
#define BSP_USING_P4_4 0
#define BSP_USING_P4_5 0
#define BSP_USING_P4_6 0
#define BSP_USING_P4_7 0
#define BSP_USING_P4_8 0
#define BSP_USING_P4_9 0
#define BSP_USING_P4_10 0
#define BSP_USING_P4_11 0
#define BSP_USING_P4_12 0
#define BSP_USING_P4_13 0
#define BSP_USING_P4_14 0
#define BSP_USING_P4_15 0

#define BSP_USING_P5_0 0
#define BSP_USING_P5_1 0
#define BSP_USING_P5_2 0
#define BSP_USING_P5_3 0
#define BSP_USING_P5_4 0
#define BSP_USING_P5_5 0
#define BSP_USING_P5_6 0
#define BSP_USING_P5_7 0
#define BSP_USING_P5_8 0
#define BSP_USING_P5_9 0
#define BSP_USING_P5_10 true
#define BSP_USING_P5_11 0
#define BSP_USING_P5_12 0
#define BSP_USING_P5_13 0
#define BSP_USING_P5_14 0
#define BSP_USING_P5_15 0

/* UART */
#define RT_USING_SERIAL

#define BSP_USING_UART2
#define BSP_UART2_BAUDRATE 115200

/* I2C */
#define RT_USING_I2C
#define BSP_USING_MIO_I2C9
#define BSP_MIO_I2C9_BAUDRATE 400000

/* SPI */
#define RT_USING_SPI
#define BSP_USING_SPI0
#define BSP_USING_SPI0_M0
#define BSP_USING_SPI0_CS_INDEX 0

/* CAN */
#define RT_USING_CAN
#define RT_CAN_USING_HDR
#define BSP_USING_CAN0
#define BSP_CAN0_BAUDRATE 1000000

/* Hardware Timer */
#define RT_USING_HWTIMER
#define BSP_USING_TIMER0

/* Ethernet net */
#define RT_USING_XMAC
#define BSP_USING_XMAC1

#define BSP_USING_XMAC3
/* 网卡收发中断的优先级 */
#define BSP_XMAC_INT_PRIO 10

#endif
