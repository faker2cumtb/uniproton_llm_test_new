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

#define RT_NAME_MAX 16
#define RT_ALIGN_SIZE 8

#define RT_USING_LIBC
#define RT_USING_DEVICE

/* GPIO */
#define RT_USING_PIN

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
#define BSP_USING_CAN1
#define BSP_CAN1_BAUDRATE 1000000

/* Hardware Timer */
#define RT_USING_HWTIMER
#define BSP_USING_TIMER1

/* Ethernet net */
#define RT_USING_XMAC
#define BSP_USING_XMAC0

#endif
