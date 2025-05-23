/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: fxmac_lwip_port.h
 * Date: 2022-11-01 14:59:30
 * LastEditTime: 2022-11-01 14:59:30
 * Description:   This file is xmac portable code for lwip port input,output,status check.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe      2022/11/3            first release
 */
#ifndef FXMAC_PORT_H
#define FXMAC_PORT_H

#include "prt_task.h"
#include "prt_typedef.h"
#include "fxmac.h"
#include "fkernel.h"
#include "ferror_code.h"
#include "prt_config.h"
#include "prt_sem.h"
#include <lwip/sys.h>
#include <lwip/err.h>
#include <rtdevice.h>

#define FREERTOS_XMAC_INIT_ERROR FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_XMAC_PARAM_ERROR FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_XMAC_NO_VALID_SPACE FT_CODE_ERR(ErrModPort, 0, 0x3)


#ifdef __cplusplus
extern "C" {
#endif

#define FXMAX_RX_BDSPACE_LENGTH     0x20000 /* default set 64KB*/
#define FXMAX_TX_BDSPACE_LENGTH     0x20000 /* default set 64KB*/

#define FXMAX_RX_PBUFS_LENGTH       128
#define FXMAX_TX_PBUFS_LENGTH       128

#define FXMAX_MAX_HARDWARE_ADDRESS_LENGTH 6

/* configuration */
#define FXMAC_PORT_CONFIG_JUMBO  BIT(0)
#define FXMAC_PORT_CONFIG_MULTICAST_ADDRESS_FILITER  BIT(1) /* Allow multicast address filtering  */
#define FXMAC_PORT_CONFIG_COPY_ALL_FRAMES BIT(2) /* enable copy all frames */
#define FXMAC_PORT_CONFIG_CLOSE_FCS_CHECK BIT(3) /* close fcs check */
#define FXMAC_PORT_CONFIG_RX_POLL_RECV BIT(4)  /* select poll mode */
#define FXMAC_PORT_CONFIG_UNICAST_ADDRESS_FILITER BIT(5) /* Allow unicast address filtering  */

/* Phy */
#define FXMAC_PHY_SPEED_10M    10
#define FXMAC_PHY_SPEED_100M    100
#define FXMAC_PHY_SPEED_1000M    1000
#define FXMAC_PHY_SPEED_10G    10000


#define FXMAC_PHY_HALF_DUPLEX   0
#define FXMAC_PHY_FULL_DUPLEX   1


/* Byte alignment of BDs */
#define BD_ALIGNMENT (FXMAC_DMABD_MINIMUM_ALIGNMENT*2)

/*  frame queue */
#define PQ_QUEUE_SIZE 4096

typedef struct 
{
    uintptr data[PQ_QUEUE_SIZE];
    int head, tail, len;
} PqQueue;

typedef enum
{
    FXMAC_PORT_INTERFACE_SGMII = 0 ,
    FXMAC_PORT_INTERFACE_RMII  ,
    FXMAC_PORT_INTERFACE_RGMII ,
    FXMAC_PORT_INTERFACE_USXGMII ,
    FXMAC_PORT_INTERFACE_LENGTH
}FXmacPortInterface;

typedef struct
{
    u8 rx_bdspace[FXMAX_RX_BDSPACE_LENGTH] __attribute__((aligned(256))); /* 接收bd 缓冲区 */
    u8 tx_bdspace[FXMAX_TX_BDSPACE_LENGTH] __attribute__((aligned(256))); /* 发送bd 缓冲区 */

    uintptr rx_pbufs_storage[FXMAX_RX_PBUFS_LENGTH];
    uintptr tx_pbufs_storage[FXMAX_TX_PBUFS_LENGTH];
    
} FXmacNetifBuffer;

typedef struct
{
    u32 instance_id;
    FXmacPortInterface interface;
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    u32 phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */
} FXmacPortConfig;

typedef struct
{
    FXmac instance;
    FXmacPortConfig xmac_port_config;

    FXmacNetifBuffer buffer;

    /* queue to store overflow packets */
	PqQueue recv_q;
	PqQueue send_q;

    /* configuration */
    u32 feature;

    void *stack_pointer; /* Docking data stack data structure */
    u8 hwaddr[FXMAX_MAX_HARDWARE_ADDRESS_LENGTH];
    SemHandle recv_sem;
    SemHandle send_sem;
    struct rt_device *device;
}FXmacPort;


enum port_link_status {
	ETH_LINK_UNDEFINED = 0,
	ETH_LINK_UP,
	ETH_LINK_DOWN,
	ETH_LINK_NEGOTIATING
};

FXmacPort * FXmacPortGetInstancePointer(u32 FXmacPortInstanceID);
FError FXmacPortInit(FXmacPort *instance_p);
void *FXmacPortRx(FXmacPort *instance_p);
FError FXmacPortTx(FXmacPort *instance_p,void *tx_buf);
void FXmacPortStop(FXmacPort *instance_p);
void FXmacPortStart(FXmacPort *instance_p);
void ResetDma(FXmacPort *instance_p);
enum port_link_status FXmacPhyReconnect(FXmacPort *instance_p);
void FXmacRecvHandler(void *arg);

#ifdef __cplusplus
}
#endif

#endif // !
