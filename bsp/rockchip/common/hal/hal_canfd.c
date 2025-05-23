/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 */

#include "hal_base.h"

#if defined(HAL_CANFD_MODULE_ENABLED)

/** @addtogroup RK_HAL_Driver
 *  @{
 */

/** @addtogroup CANFD
 *  @{
 */

/** @defgroup CANFD_How_To_Use How To Use
 *  @{

 The CANFD driver can be used as follows:

 - Init: set work mode: HAL_CANFD_Init()

 - Start: start can bus: HAL_CANFD_Start()

 - Stop: stop can bus: HAL_CANFD_Stop()

 - Tx: HAL_CANFD_Transmit()

 - Rx: HAL_CANFD_Receive()

 @} */

/** @defgroup CANFD_Private_Definition Private Definition
 *  @{
 */
/********************* Private MACRO Definition ******************************/
/********************* Private Structure Definition **************************/
/********************* Private Variable Definition ***************************/

static const uint8_t dlc2len[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

static const uint8_t len2dlc[] = {0, 1, 2, 3, 4, 5, 6, 7, 8,       /* 0 - 8 */
                                  9, 9, 9, 9,                      /* 9 - 12 */
                                  10, 10, 10, 10,                  /* 13 - 16 */
                                  11, 11, 11, 11,                  /* 17 - 20 */
                                  12, 12, 12, 12,                  /* 21 - 24 */
                                  13, 13, 13, 13, 13, 13, 13, 13,  /* 25 - 32 */
                                  14, 14, 14, 14, 14, 14, 14, 14,  /* 33 - 40 */
                                  14, 14, 14, 14, 14, 14, 14, 14,  /* 41 - 48 */
                                  15, 15, 15, 15, 15, 15, 15, 15,  /* 49 - 56 */
                                  15, 15, 15, 15, 15, 15, 15, 15}; /* 57 - 64 */

/********************* Private Function Definition ***************************/

/**
 * @brief Get data length from can_dlc with sanitized can_dlc.
 * @param  dlc: can_dlc
 * @return length.
 */
static uint8_t CANFD_Dlc2Len(uint8_t dlc)
{
    return dlc2len[dlc & 0x0F];
}

/**
 * @brief Map the sanitized data length to an appropriate data length code.
 * @param  len: can length
 * @return dlc.
 */
static uint8_t CANFD_Len2Dlc(uint8_t len)
{
    if (len > 64)
    {
        return 0xF;
    }

    return len2dlc[len];
}

/**
 * @brief Set can to reset mode.
 * @param  pReg: can base
 * @return HAL_OK.
 */
static HAL_Status CANFD_SetResetMode(struct CAN_REG *pReg)
{
    pReg->MODE = 0;
    WRITE_REG(pReg->INT_MASK0, 0xffff);

    return HAL_OK;
}

/**
 * @brief Set can to normal mode.
 * @param  pReg: can base
 * @return HAL_OK.
 */
static HAL_Status CANFD_SetNormalMode(struct CAN_REG *pReg)
{
    SET_BIT(pReg->MODE, CAN_MODE_WORK_MODE_MASK);

    return HAL_OK;
}

/**
 * @brief Set CAN Filters, Mode and BitTiming
 * @param  pReg: can base
 * @param  initConfig: can init parameter
 * @return HAL_OK
 */
static HAL_Status HAL_CANFD_SetConfig(struct CAN_REG *pReg, struct CANFD_CONFIG *initConfig)
{
    const uint32_t mask = 0x1FFFFFFFU;

    pReg->AFR_CTRL = 0x1f;
    pReg->IDCODE0 = initConfig->canfdFilterId[0];
    pReg->IDMASK0 = initConfig->canfdFilterMask[0] & mask;
    pReg->IDCODE1 = initConfig->canfdFilterId[1];
    pReg->IDMASK1 = initConfig->canfdFilterMask[1] & mask;
    pReg->IDCODE2 = initConfig->canfdFilterId[2];
    pReg->IDMASK2 = initConfig->canfdFilterMask[2] & mask;
    pReg->IDCODE3 = initConfig->canfdFilterId[3];
    pReg->IDMASK3 = initConfig->canfdFilterMask[3] & mask;
    pReg->IDCODE4 = initConfig->canfdFilterId[4];
    pReg->IDMASK4 = initConfig->canfdFilterMask[4] & mask;

    pReg->IDCODE = initConfig->canfdFilterId[5];
    pReg->IDMASK = initConfig->canfdFilterMask[5] & mask;

    if ((initConfig->canfdMode & CANFD_MODE_LOOPBACK) == CANFD_MODE_LOOPBACK)
    {
        SET_BIT(pReg->MODE, CAN_MODE_LBACK_MODE_MASK | CAN_MODE_SELF_TEST_MASK);
    }
    if ((initConfig->canfdMode & CANFD_MODE_LISTENONLY) == CANFD_MODE_LISTENONLY)
    {
        SET_BIT(pReg->MODE, CAN_MODE_SILENT_MODE_MASK);
    }
    if ((initConfig->canfdMode & CANFD_MODE_3_SAMPLES) == CANFD_MODE_3_SAMPLES)
    {
        SET_BIT(pReg->BITTIMING, CAN_BITTIMING_SAMPLE_MODE_MASK);
    }
    else
    {
        CLEAR_BIT(pReg->BITTIMING, CAN_BITTIMING_SAMPLE_MODE_MASK);
    }

    SET_BIT(pReg->MODE, CAN_MODE_AUTO_RETX_MODE_MASK | CAN_MODE_AUTO_BUS_ON_MASK);

    return HAL_OK;
}

/**
 * @brief CANFD get normal bps.
 * @param  pReg: can base
 * @param  bps: can BaudRate
 * @return HAL_OK.
 * How to calculate the can bit rate:
 *     BaudRate= clk_can/(2*(brq+1)/(1+(tseg1+1)+(tseg2+1)))
 *     TDC = 0
 *     For example(clk_can=200M):
 *     CLK_CAN    BPS    BRQ    TSEG1    TSEG2    TDC
 *     200M       1M     4      13       4        0
 *     200M       500K   9      13       4        0
 *     200M       250K   19     13       4        0
 *     200M       200K   24     13       4        0
 *     200M       125K   39     13       4        0
 *     200M       100K   49     13       4        0
 */
HAL_Status HAL_CANFD_SetBps(struct CAN_REG *pReg, eCANFD_Bps bps)
{
    uint32_t sjw, brq, tseg1, tseg2;

    HAL_ASSERT(IS_CAN_INSTANCE(pReg));
    switch (bps)
    {
    case CANFD_BPS_1MBAUD:
        brq = 4;
        tseg1 = 13;
        tseg2 = 4;
        break;
    case CANFD_BPS_500KBAUD:
        brq = 9;
        tseg1 = 13;
        tseg2 = 4;
        break;
    case CANFD_BPS_250KBAUD:
        brq = 19;
        tseg1 = 13;
        tseg2 = 4;
        break;
    case CANFD_BPS_200KBAUD:
        brq = 24;
        tseg1 = 13;
        tseg2 = 4;
        break;
    case CANFD_BPS_125KBAUD:
        brq = 39;
        tseg1 = 13;
        tseg2 = 9;
        break;
    case CANFD_BPS_100KBAUD:
        brq = 49;
        tseg1 = 13;
        tseg2 = 4;
        break;
    default:
        return HAL_INVAL;
    }

    sjw = 0;

    CLEAR_BIT(pReg->BITTIMING,
              CAN_BITTIMING_SJW_MASK |
              CAN_BITTIMING_BRP_MASK |
              CAN_BITTIMING_TSEG1_MASK |
              CAN_BITTIMING_TSEG2_MASK);

    SET_BIT(pReg->BITTIMING,
            (sjw << CAN_BITTIMING_SJW_SHIFT) |
            (brq << CAN_BITTIMING_BRP_SHIFT) |
            (tseg1 << CAN_BITTIMING_TSEG1_SHIFT) |
            (tseg2 << CAN_BITTIMING_TSEG2_SHIFT));

    return HAL_OK;
}


/** @} */
/********************* Public Function Definition ****************************/

/** @defgroup CANFD_Exported_Functions_Group5 Other Functions
 *  @{
 */

/**
 * @brief CANFD init.
 * @param  pReg: can base
 * @param  initConfig: can init parameter
 * @return HAL_OK.
 */
HAL_Status HAL_CANFD_Init(struct CAN_REG *pReg, struct CANFD_CONFIG *initConfig)
{
    uint32_t mask = 0;

    HAL_ASSERT(IS_CAN_INSTANCE(pReg));
    HAL_ASSERT(initConfig != NULL);

    CANFD_SetResetMode(pReg);

    mask = HAL_CANFD_GetErrInterruptMaskCombin(CANFD_INT_ERR);
    mask |= HAL_CANFD_GetErrInterruptMaskCombin(CANFD_INT_RX_OF);
    mask |= HAL_CANFD_GetErrInterruptMaskCombin(CANFD_INT_TX_FINISH);
    mask |= HAL_CANFD_GetErrInterruptMaskCombin(CANFD_INT_RX_FINISH);
    pReg->INT_MASK0 = ~mask;

    //pReg->RX_FIFO_CTRL |= CAN_RX_FIFO_CTRL_RX_FIFO_ENABLE_MASK;

    HAL_CANFD_SetConfig(pReg, initConfig);
    HAL_CANFD_SetBps(pReg, initConfig->bps);

    return HAL_OK;
}

/**
 * @brief  Clear CAN interrupt status.
 * @param  pReg: Choose CAN.
 * @return HAL_Status: HAL_OK.
 */
HAL_Status HAL_CANFD_ClrInt(struct CAN_REG *pReg, const uint32_t isr)
{
    uint32_t timeOut = 1000;

    HAL_ASSERT(IS_CAN_INSTANCE(pReg));

    pReg->INT = isr;
    while (pReg->INT && timeOut)
    {
        timeOut--;
    }

    if (timeOut == 0)
    {
        return HAL_TIMEOUT;
    }
    else
    {
        return HAL_OK;
    }
}

/**
 * @brief CANFD start.
 * @param  pReg: can base
 * @return HAL_OK.
 */
HAL_Status HAL_CANFD_Start(struct CAN_REG *pReg)
{
    HAL_ASSERT(IS_CAN_INSTANCE(pReg));

    CANFD_SetNormalMode(pReg);

    return HAL_OK;
}

/**
 * @brief CANFD stop.
 * @param  pReg: can base
 * @return HAL_OK.
 */
HAL_Status HAL_CANFD_Stop(struct CAN_REG *pReg)
{
    HAL_ASSERT(IS_CAN_INSTANCE(pReg));

    CANFD_SetResetMode(pReg);

    return HAL_OK;
}

/**
 * @brief CANFD tx.
 * @param  pReg: can base
 * @param  TxMsg: Tx message
 * @return HAL_OK.
 */
HAL_Status HAL_CANFD_Transmit(struct CAN_REG *pReg, struct CANFD_MSG *TxMsg)
{
    uint8_t cmd = CAN_CMD_TX0_REQ_MASK;

    HAL_ASSERT(IS_CAN_INSTANCE(pReg));
    HAL_ASSERT(TxMsg != NULL);

    const uint32_t can_cmd = READ_REG(pReg->CMD);
    const uint32_t tx0_req_mask = (can_cmd & CAN_CMD_TX0_REQ_MASK);
    const uint32_t tx1_req_mask = (can_cmd & CAN_CMD_TX1_REQ_MASK);

    if (tx0_req_mask && tx1_req_mask)
    {
        return HAL_BUSY;
    }

    if (tx0_req_mask)
    {
        cmd = CAN_CMD_TX1_REQ_MASK;
    }

    uint32_t txframeinfo = 0;
    uint32_t txid = TxMsg->stdId;

    if (TxMsg->ide == CANFD_ID_EXTENDED)
    {
        txid = TxMsg->extId;
        txframeinfo |= CAN_TXFRAMEINFO_TXFRAME_FORMAT_MASK;
    }

    if (TxMsg->rtr == CANFD_RTR_REMOTE)
    {
        TxMsg->dlc = 0;
        txframeinfo |= CAN_TXFRAMEINFO_TX_RTR_MASK;
    }

    txframeinfo |= (CANFD_Len2Dlc(TxMsg->dlc) & CAN_TXFRAMEINFO_TXDATA_LENGTH_MASK);

    if (cmd == CAN_CMD_TX0_REQ_MASK)
    {
        WRITE_REG(pReg->TXID, txid);
        WRITE_REG(pReg->TXFRAMEINFO, txframeinfo);
        pReg->TXDATA0 = *(uint32_t *)&TxMsg->data[0];
        pReg->TXDATA1 = *(uint32_t *)&TxMsg->data[4];
    } else {
        WRITE_REG(pReg->FD_TXID, txid);
        WRITE_REG(pReg->FD_TXFRAMEINFO, txframeinfo);
        pReg->FD_TXDATA[0] = *(uint32_t *)&TxMsg->data[0];
        pReg->FD_TXDATA[1] = *(uint32_t *)&TxMsg->data[4];
    }
    WRITE_REG(pReg->CMD, cmd);

    return HAL_OK;
}

/**
 * @brief CANFD rx.
 * @param  pReg: can base
 * @param  RxMsg: Rx message
 * @return HAL_OK.
 */
HAL_Status HAL_CANFD_Receive(struct CAN_REG *pReg, struct CANFD_MSG *RxMsg)
{
    uint32_t info, id, len, data[16];

    HAL_ASSERT(IS_CAN_INSTANCE(pReg));

    int ret = HAL_OK;
    if(!(pReg->STATE & CAN_STATE_RX_BUFFER_FULL_MASK))
    {
        ret = HAL_ERROR;
    }

    info = READ_REG(pReg->RXFRAMEINFO);
    id = READ_REG(pReg->RXID);

    data[0] = READ_REG(pReg->RXDATA0);
    data[1] = READ_REG(pReg->RXDATA1);

    len = info & CAN_RXFRAMEINFO_RXDATA_LENGTH_MASK;
    RxMsg->ide = (info & CAN_RXFRAMEINFO_RXFRAME_FORMAT_MASK) >> CAN_RXFRAMEINFO_RXFRAME_FORMAT_SHIFT;
    RxMsg->rtr = (info & CAN_RXFRAMEINFO_RX_RTR_MASK) >> CAN_RXFRAMEINFO_RX_RTR_SHIFT;
    RxMsg->dlc = CANFD_Dlc2Len(len);
    if (RxMsg->ide == CANFD_ID_EXTENDED)
    {
        RxMsg->extId = id & CAN_FD_RXID_RX_ID_MASK;
    }
    else
    {
        RxMsg->stdId = id & 0x7FFU;
    }

    *(uint32_t *)&RxMsg->data[0] = data[0];
    *(uint32_t *)&RxMsg->data[4] = data[1];

    return ret;
}

/**
 * @brief CANFD get interrupt.
 * @param  pReg: can base
 * @return interrupt status.
 */
uint32_t HAL_CANFD_GetInterrupt(struct CAN_REG *pReg)
{
    uint32_t isr;

    HAL_ASSERT(IS_CAN_INSTANCE(pReg));

    isr = READ_REG(pReg->INT);

    /* set 1 to clear interrupt */
    WRITE_REG(pReg->INT, isr);

    return isr;
}

/**
 * @brief CANFD get interrupt mask combin.
 * @param type: interrupt type
 * @return err interrupt mask combin.
 */
uint32_t HAL_CANFD_GetErrInterruptMaskCombin(eCANFD_IntType type)
{
    uint32_t isr = 0;

    switch (type)
    {
    case CANFD_INT_ERR:
        isr = CAN_INT_BUS_OFF_INT_MASK |
              CAN_INT_ERROR_INT_MASK |
              CAN_INT_TX_ARBIT_FAIL_INT_MASK |
              CAN_INT_PASSIVE_ERROR_INT_MASK |
              CAN_INT_OVERLOAD_INT_MASK |
              CAN_INT_ERROR_WARNING_INT_MASK;
        break;
    case CANFD_INT_RX_OF:
        isr = CAN_INT_RX_FIFO_OVERFLOW_INT_MASK | CAN_INT_RX_FIFO_FULL_INT_MASK;
        break;
    case CANFD_INT_TX_FINISH:
        isr = CAN_INT_TX_FINISH_INT_MASK;
        break;
    case CANFD_INT_RX_FINISH:
        isr = CAN_INT_RX_FINISH_INT_MASK;
        break;
    default:
        break;
    }

    return isr;
}

/** @} */

/** @} */

/** @} */

#endif
