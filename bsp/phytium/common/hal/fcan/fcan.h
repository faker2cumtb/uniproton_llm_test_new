/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: fcan.h
 * Date: 2021-04-27 15:08:44
 * LastEditTime: 2022-02-18 08:29:25
 * Description:  This files is for the can function related definitions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/5/26  first release
 * 1.1   wangxiaodong  2022/9/23  improve functions
 * 1.2   zhangyan      2022/12/7  improve functions
 * 1.3   huangjin      2023/11/01 improve functions
 * 1.4   huangjin      2024/5/24 improve functions
 */


#ifndef FCAN_H
#define FCAN_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fparameters.h"
#include "fcan_hw.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    FCAN_INTR_EVENT_SEND = 0,     /* Handler type for frame sending interrupt */
    FCAN_INTR_EVENT_RECV = 1,     /* Handler type for frame reception interrupt */
    FCAN_INTR_EVENT_ERROR = 2,    /* Handler type for error interrupt */
    FCAN_INTR_EVENT_BUSOFF = 3,   /* Handler type for bus off interrupt */
    FCAN_INTR_EVENT_PERROE = 4,   /* Handler type for passion error interrupt */
    FCAN_INTR_EVENT_PWARN = 5,    /* Handler type for passion warn interrupt */
    FCAN_INTR_EVENT_FIFOFULL = 6, /* Handler type for rx fifo register full */
    FCAN_INTR_EVENT_FIFOEMPTY = 7,/* Handler type for tx fifo register empty */
    FCAN_INTR_EVENT_NUM
} FCanIntrEventType;

#define FCAN_SUCCESS            FT_SUCCESS /* SUCCESS */
#define FCAN_NOT_READY          FT_MAKE_ERRCODE(ErrModBsp, ErrBspCan, 1)
#define FCAN_FAILURE            FT_MAKE_ERRCODE(ErrModBsp, ErrBspCan, 2)  /* failed */
#define FCAN_INVAL_PARAM        FT_MAKE_ERRCODE(ErrModBsp, ErrBspCan, 3)  /* invalid parameters */
#define FCAN_TX_FIFO_FULL_ERR   FT_MAKE_ERRCODE(ErrModBsp, ErrBspCan, 4)  /* tx fifo full */
#define FCAN_RX_FIFO_EMPTY_ERR  FT_MAKE_ERRCODE(ErrModBsp, ErrBspCan, 5)  /* rx fifo empty */

#define FCAN_DATA_LENGTH 64U

/* CAN payload length and DLC definitions according to ISO 11898-1 */
#define CAN_MAX_DLC     8
#define CAN_MAX_DLEN    8
#define CAN_MAX_CTL     3
#define CAN_SFF_ID_BITS 11
#define CAN_EFF_ID_BITS 29

/* special address description flags for the CAN_ID */
#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error message frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

/* Frame type */
#define STANDARD_FRAME  0   /* standard frame */
#define EXTEND_FRAME    1   /* extended frame */

/* Bit timing calculate */
#define CAN_CALC_MAX_ERROR  50 /* in one-tenth of a percent */
#define CAN_CALC_SYNC_SEG   1

/*Transmit mode*/
#define FCAN_PROBE_MONITOR_MODE 0 /* Monitor mode */
#define FCAN_PROBE_NORMAL_MODE  1 /* Normal mode */

/* can segment type */
typedef enum
{
    FCAN_ARB_SEGMENT = 0, /* Arbitration segment */
    FCAN_DATA_SEGMENT = 1, /* Data segment */
    FCAN_SEGMENT_TYPE_NUM
} FCanSegmentType;

/* Can error status bit mask */
#define FCAN_BUS_ERROR_MASK         1
#define FCAN_PASSIVE_ERROR_MASK     2
#define FCAN_PASSIVE_WARNING_MASK   4
#define FCAN_FIFO_RX_OVERFLOW_MASK  8

/* Can frame select */
#define FCAN_STANDARD_FRAME     0
#define FCAN_EXTENDARD_FRAME    1

/* can baudrate */
#define FCAN_BAUDRATE_10K    10000
#define FCAN_BAUDRATE_50K    50000
#define FCAN_BAUDRATE_100K   100000
#define FCAN_BAUDRATE_200K   200000
#define FCAN_BAUDRATE_250K   250000
#define FCAN_BAUDRATE_500K   500000
#define FCAN_BAUDRATE_1000K  1000000
#define FCAN_BAUDRATE_2000K  2000000
#define FCAN_BAUDRATE_3000K  3000000
#define FCAN_BAUDRATE_4000K  4000000
#define FCAN_BAUDRATE_5000K  5000000

/*
 * defined bits for FCanFrame.flags
 *
 * The use of struct FCanFrame implies the Extended Data Length (EDL) bit to
 * be set in the CAN frame bitstream on the wire. The EDL bit switch turns
 * the CAN controllers bitstream processor into the CAN FD mode which creates
 * two new options within the CAN FD frame specification:
 *
 * Bit Rate Switch - to indicate a second baudrate is/was used for the payload
 * Error State Indicator - represents the error state of the transmitting node
 *
 * As the CANFD_ESI bit is internally generated by the transmitting CAN
 * controller only the CANFD_BRS bit is relevant for real CAN controllers when
 * building a CAN FD frame for transmission. Setting the CANFD_ESI bit can make
 * sense for virtual CAN interfaces to test applications with echoed frames.
 */
#define CANFD_BRS 0x02 /* bit rate switch (second baudrate for payload data) */
#define CANFD_ESI 0x04 /* error state indicator of the transmitting node */

typedef void (*FCanIntrEventHandler)(void *param);

typedef struct
{
    FCanIntrEventType type;
    FCanIntrEventHandler handler;
    void *param;
} FCanIntrEventConfig;

typedef struct
{
    u32 filter_index;/* filter register index*/
    u32 id; /* id bit to receive */
    u32 mask;/* id mask bit to receive */
    u32 type;/* frame type, standard or extended*/
} FCanIdMaskConfig;

typedef struct
{
    u32 canid;/* can frame id */
    u8 candlc;/* can frame length */
    u8 flags; /* additional flags for CAN FD */
    u8 data[FCAN_DATA_LENGTH] __attribute__((aligned(8)));
} FCanFrame;

typedef struct
{
    u32 instance_id;   /* Id of device */
    uintptr base_address; /* Can base Address */
    u32 irq_num;    /* interrupt number */
    u32 irq_prority;/* interrupt priority*/
    u32 caps;
} FCanConfig;

typedef struct
{
    FCanSegmentType segment;
    boolean auto_calc; /* if auto calculate baudrate parameters */
    u32 baudrate;     /* baudrate */
    u32 sample_point; /* sample point */
    u32 prop_seg;     /* Propagation segment in TQs */
    u32 phase_seg1;   /* Phase buffer segment 1 in TQs */
    u32 phase_seg2;   /* Phase buffer segment 2 in TQs */
    u32 sjw;          /* Synchronisation jump width in TQs */
    u32 brp;          /* Baudrate prescaler */
} FCanBaudrateConfig;

typedef struct
{
    u8 xfers;/* transfer status */
    u8 rs;   /* receive status */
    u8 ts;   /* transmit status */
    u8 fies; /* Current status of the controller state machine */
    u8 fras; /* Frame tagging status */
} FCanXferStatus;

typedef struct
{
    FCanXferStatus xfer_status;
    u32 tx_err_cnt;
    u32 rx_err_cnt;
    u32 tx_fifo_cnt;
    u32 rx_fifo_cnt;
} FCanStatus;

typedef struct
{
    FCanConfig config;
    u32 is_ready;  /* Device is initialized and ready */
    boolean use_canfd; /* if use canfd function */

    FCanIntrEventConfig intr_event[FCAN_INTR_EVENT_NUM];/* event handler and parameters for interrupt */
} FCanCtrl;

/* get default configuration of specific can id */
const FCanConfig *FCanLookupConfig(u32 instance_id);

/* reset a specific can instance */
void FCanReset(FCanCtrl *instance_p);

/* Deinitializes a specific can instance  */
void FCanDeInitialize(FCanCtrl *instance_p);

/* Initializes a specific can instance  */
FError FCanCfgInitialize(FCanCtrl *instance_p, const FCanConfig *input_config_p);

/* Set the fcan baudrate */
FError FCanBaudrateSet(FCanCtrl *instance_p, FCanBaudrateConfig *baudrate_p);

/* interrupt handler for the driver */
void FCanIntrHandler(s32 vector, void *args);

/* register FCanCtrl interrupt handler function */
void FCanRegisterInterruptHandler(FCanCtrl *instance_p, FCanIntrEventConfig *intr_event_p);

/* receive can message by specific can instance */
FError FCanRecv(FCanCtrl *instance_p, FCanFrame *frame_p);

/* convert data frames to buf */
u32 FCanFrameToBuf(FCanCtrl *instance_p, FCanFrame *frame_p, u32 *buf);

/* send can message by specific can instance */
FError FCanSend(FCanCtrl *instance_p, FCanFrame *frame_p);

/* Enable the specific can instance */
FError FCanEnable(FCanCtrl *instance_p, boolean enable);

/* read can status, include send and receive error count */
FError FCanStatusGet(FCanCtrl *instance_p, FCanStatus *status_p);

/* Set the can mask and umask id */
FError FCanIdMaskFilterSet(FCanCtrl *instance_p, FCanIdMaskConfig *id_mask_p);

/* Set the can id mask filter enable */
void FCanIdMaskFilterEnable(FCanCtrl *instance_p);

/* Set the can id mask filter disable */
void FCanIdMaskFilterDisable(FCanCtrl *instance_p);

/* Enable can interrupt */
FError FCanInterruptEnable(FCanCtrl *instance_p, FCanIntrEventType event_type);

/* Enable or disable can fd */
FError FCanFdEnable(FCanCtrl *instance_p, boolean enable);

/* Set the transmit mode  */
FError FCanSetMode(FCanCtrl *instance_p, u32 tran_mode);

/* Enable or disable can dma */
void FCanDmaEnable(FCanCtrl *instance_p, u32 enable_mode);

#ifdef __cplusplus
}
#endif

#endif // !FT_CAN_H
