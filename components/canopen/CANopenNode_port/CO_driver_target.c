/**
 *******************************************************************************
 *
 * @file CO_driver_target.c
 * @author shianhu@ncti-gba.cn
 * @brief CAN module object for CAN peripheral.
 *
 * @attention Modified with permission
 *
 * @date Created on March 18, 2024
 *
 *******************************************************************************
 * @copyright Copyright (c) 2024 NCTI
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *******************************************************************************
 */

#include "CO_driver_target.h"
#include "CO_app_target.h"
#include "prt_sys.h"
#include "rtdevice.h"


// Global variables and objects
// Global pointer to the instance of CAN module and will be set by CO_CANmodule_init()
static CO_CANmodule_t *g_CANmodule_p = NULL;

// Get error counters from the module
// If necessary, function may use different way to determine errors
static uint16_t rxErrors = 0U, txErrors = 0U, overflow = 0U;


/**
 * @brief Send CAN message to network
 *        This function must be called with atomic access
 *        NOTE: Must lock interrupts for atomic operation by
 *              calling CO_LOCK_CAN_SEND()
 *
 * @param[in] CANmodule CAN module instance
 * @param[in] buffer    Pointer to buffer to transmit
 *
 * @return uint8_t
 */
static uint8_t prv_send_can_message(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer)
{
    uint8_t send_rc = 1U;

    CO_CANtxMsg_t txMsg = {0};

    txMsg.id = buffer->ident & CAN_ID_STD_MASK;
    txMsg.ide = RT_CAN_STDID;
    txMsg.rtr = (buffer->ident & FLAG_RTR) ? RT_CAN_RTR : RT_CAN_DTR;
    txMsg.len = buffer->DLC;
    memcpy(txMsg.data, buffer->data, sizeof(txMsg.data));

    rt_size_t rt_rv = rt_device_write(((CANopenNodeRt*)CANmodule->CANptr)->canHandle,
                                      0, &txMsg, sizeof(txMsg));
    if (rt_rv != sizeof(txMsg))
    {
        PRT_Printf("ERROR: rt_device_write() failed, rt_rv=%d, size=%zu\n", rt_rv, sizeof(txMsg));
    }
    else
    {
        send_rc = 0U;
    }

    return send_rc;
}


// Receive interrupt
static void prv_CO_CANinterruptRx(CO_CANmodule_t *CANmodule)
{
    CO_CANrxMsg_t rxMsg = {0};
    // 因驱动实现问题，此处必须设为-1，表示直接从uselist链表读取数据
    rxMsg.hdr_index = -1;

    rt_size_t rt_rv = rt_device_read(((CANopenNodeRt*)CANmodule->CANptr)->canHandle,
                                     0, &rxMsg, sizeof(rxMsg));
    if (rt_rv != sizeof(rxMsg))
    {
        PRT_Printf("ERROR: rt_device_read() failed, rt_rv=%d, size=%zu\n", rt_rv, sizeof(rxMsg));
        return;
    }

    CO_CANrx_t *buffer = NULL; // Receive message buffer from CO_CANmodule_t object
    bool_t msgMatched = false;
    // Identifier of the received message
    uint32_t rcvMsgIdent = rxMsg.id | (rxMsg.rtr == RT_CAN_RTR ? FLAG_RTR : 0U);

    if (CANmodule->useCANrxFilters)
    {
        // CAN module filters are used.
        // Message with known 11-bit identifier has been received.
    }
    else
    {
        // CAN module filters are not used, message with any standard 11-bit
        // identifier has been received.
        // Search rxArray form CANmodule for the same CAN-ID.
        buffer = &CANmodule->rxArray[0];

        for (uint16_t i = CANmodule->rxSize; i > 0U; i--)
        {
            if (((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U)
            {
                msgMatched = true;
                break;
            }

            buffer++;
        }
    }

    // Call the callback function, which will process the message
    if (msgMatched && (buffer != NULL) && (buffer->CANrx_callback != NULL))
    {
        buffer->CANrx_callback(buffer->object, (void*)&rxMsg);
    }
    else
    {
        PRT_Printf("ERROR: Can not find handler for this message, rcvMsgIdent=0x%X\n",
                   rcvMsgIdent);
        PRT_SysReboot();
    }

    // Clear interrupt flag
}


// Transmit interrupt
/* Try to send more buffers, process all empty ones
 *
 * This function is always called from interrupt,
 * however to make sure no preemption can happen, interrupts are anyway locked
 * (unless you can guarantee no higher priority interrupt will try to access to
 * CAN instance and send data, then no need to lock interrupts.)
 */
static void prv_CO_CANinterruptTx(CO_CANmodule_t *CANmodule)
{
    // Clear interrupt flag

    // Send message to CAN network, Lock interrupts for atomic operation
    CO_LOCK_CAN_SEND(CANmodule);

    // First CAN message (bootup) was sent successfully
    CANmodule->firstCANtxMessage = false;

    // Clear flag from previous message
    CANmodule->bufferInhibitFlag = false;

    // Are there any new messages waiting to be send
    if (CANmodule->CANtxCount > 0U)
    {
        uint16_t i = 0U; ///< index of transmitting message
        // Set to first buffer
        CO_CANtx_t *buffer = &CANmodule->txArray[0];

        // Search through whole array of pointers to transmit message buffers
        for (i = CANmodule->txSize; i > 0U; i--)
        {
            // If message buffer is full, send it
            if (buffer->bufferFull)
            {
                if (!prv_send_can_message(CANmodule, buffer))
                {
                    buffer->bufferFull = false;
                    if (CANmodule->CANtxCount > 0U)
                    {
                        CANmodule->CANtxCount--;
                    }
                    CANmodule->bufferInhibitFlag = buffer->syncFlag;
                }
                else
                {
                    PRT_Printf("WARNING: Can not send message, buffer=%p, ident=0x%X, bufferFull=%d",
                               buffer, buffer->ident, buffer->bufferFull);
                }
                break;
            }

            buffer++;
        }

        // Clear counter if no more messages
        if (i == 0U)
        {
            CANmodule->CANtxCount = 0U;
        }
    }

    CO_UNLOCK_CAN_SEND(CANmodule);
}


void CO_CANopenRx_Isr()
{
    prv_CO_CANinterruptRx(g_CANmodule_p);
}


void CO_CANopenTx_Isr()
{
    prv_CO_CANinterruptTx(g_CANmodule_p);
}


// Put CAN module in configuration mode
void CO_CANsetConfigurationMode(void *CANptr)
{
    if (CANptr != NULL)
    {
        rt_err_t rt_rc = rt_device_close(((CANopenNodeRt*)CANptr)->canHandle);
        if (rt_rc != RT_EOK)
        {
            PRT_Printf("ERROR: rt_device_close() failed, rt_rc=%d\n", rt_rc);
            PRT_SysReboot();
        }
    }
}


// Put CAN module in normal mode
void CO_CANsetNormalMode(CO_CANmodule_t *CANmodule)
{
    if (CANmodule->CANptr != NULL)
    {
        rt_err_t rt_rc = rt_device_open(((CANopenNodeRt*)CANmodule->CANptr)->canHandle,
                                        RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
        if (rt_rc != RT_EOK)
        {
            PRT_Printf("ERROR: rt_device_open() failed, rt_rc=%d\n", rt_rc);
            PRT_SysReboot();
        }
    }
}


CO_ReturnError_t CO_CANmodule_init(CO_CANmodule_t *CANmodule,
                                   void *CANptr,
                                   CO_CANrx_t rxArray[],
                                   uint16_t rxSize,
                                   CO_CANtx_t txArray[],
                                   uint16_t txSize,
                                   uint16_t CANbitRate)
{
    (void)CANbitRate;
    // Verify arguments
    if (CANmodule == NULL || rxArray == NULL || txArray == NULL)
    {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }

    // Keep a local copy of CANModule
    g_CANmodule_p = CANmodule;

    // Configure object variables
    CANmodule->CANptr = CANptr; ///< Hold CANModule variable
    CANmodule->rxArray = rxArray;
    CANmodule->rxSize = rxSize;
    CANmodule->txArray = txArray;
    CANmodule->txSize = txSize;
    CANmodule->CANerrorStatus = 0U;
    CANmodule->CANnormal = false;
    /* Do not use HW filters.
     * Microcontroller specific.
     */
    CANmodule->useCANrxFilters = false;
    CANmodule->bufferInhibitFlag = false;
    CANmodule->firstCANtxMessage = true;
    CANmodule->CANtxCount = 0U;
    CANmodule->errOld = 0U;

    // Reset all variables
    for (uint16_t i = 0U; i < rxSize; i++)
    {
        rxArray[i].ident = 0U;
        rxArray[i].mask = 0xFFFFU;
        rxArray[i].object = NULL;
        rxArray[i].CANrx_callback = NULL;
    }

    for (uint16_t i = 0U; i < rxSize; i++)
    {
        txArray[i].bufferFull = false;
    }

    // Configure CAN module registers

    // Configure CAN timing

    // Configure CAN module hardware filters
    if (CANmodule->useCANrxFilters)
    {
        /* CAN module filters are used, they will be configured with
         * CO_CANrxBufferInit(), called by separate CANopen init functions.
         * Configure all masks so that received message must match filter
         */
    }
    else
    {
        /* CAN module filters are not used, all messages with standard 11-bit
         * identifier will be received.
         * Configure mask 0 so that all messages with standard identifier are accepted
         */
    }

    // Configure CAN interrupt registers

    // Enable notifications

    // Hardware related configuration
    ((CANopenNodeRt*)CANptr)->hwInitFunction(((CANopenNodeRt*)CANptr)->canHandle,
                                              ((CANopenNodeRt*)CANptr)->canBaudRate,
                                              ((CANopenNodeRt*)CANptr)->timerHandle);

    return CO_ERROR_NO;
}


void CO_CANmodule_disable(CO_CANmodule_t *CANmodule)
{
    if ((CANmodule != NULL) && (CANmodule->CANptr != NULL))
    {
        // Turn off the module
        rt_err_t rt_rc = rt_device_close(((CANopenNodeRt*)CANmodule->CANptr)->canHandle);
        if (rt_rc != RT_EOK)
        {
            PRT_Printf("ERROR: rt_device_close() failed, rt_rc=%d\n", rt_rc);
            PRT_SysReboot();
        }
    }
}


CO_ReturnError_t CO_CANrxBufferInit(CO_CANmodule_t *CANmodule,
                                    uint16_t index,
                                    uint16_t ident,
                                    uint16_t mask,
                                    bool_t rtr,
                                    void *object,
                                    void (*CANrx_callback)(void*, void*))
{
    CO_ReturnError_t ret = CO_ERROR_NO;

    if ((CANmodule != NULL) && (object != NULL) && (CANrx_callback != NULL) &&
        (index < CANmodule->rxSize))
    {
        // Get the buffer which will be configured
        CO_CANrx_t *buffer = &CANmodule->rxArray[index];

        // Configure object variables
        buffer->object = object;
        buffer->CANrx_callback = CANrx_callback;

        /* CAN identifier and CAN mask, bit aligned with CAN module, including RTR bit.
         * Microcontroller specific.
         * This is used for RX operation match case later
         */
        buffer->ident = (ident & CAN_ID_STD_MASK) | (rtr ? FLAG_RTR : 0U);
        buffer->mask = (mask & CAN_ID_STD_MASK) | FLAG_RTR;

        // Set CAN hardware module filter and mask if necessary
    }
    else
    {
        ret = CO_ERROR_ILLEGAL_ARGUMENT;
    }

    return ret;
}


CO_CANtx_t *CO_CANtxBufferInit(CO_CANmodule_t *CANmodule,
                               uint16_t index,
                               uint16_t ident,
                               bool_t rtr,
                               uint8_t noOfBytes,
                               bool_t syncFlag)
{
    CO_CANtx_t *buffer = NULL;

    if ((CANmodule != NULL) && (index < CANmodule->txSize))
    {
        // Get the buffer which will be configured
        buffer = &CANmodule->txArray[index];

        /* CAN identifier, RTR and DLC, bit aligned with CAN module transmit buffer.
         * Microcontroller specific.
         */
        buffer->ident = ((uint32_t)ident & CAN_ID_STD_MASK) |
                        ((uint32_t)(rtr ? FLAG_RTR : 0U));
        buffer->DLC = noOfBytes;
        buffer->bufferFull = false;
        buffer->syncFlag = syncFlag;
    }

    return buffer;
}


CO_ReturnError_t CO_CANsend(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer)
{
    CO_ReturnError_t err = CO_ERROR_NO;

    // Send message to CAN network, Lock interrupts for atomic operation
    CO_LOCK_CAN_SEND(CANmodule);

    // Verify overflow
    if (buffer->bufferFull)
    {
        if (!CANmodule->firstCANtxMessage)
        {
            // Don not set error, if bootup message is still on buffers
            CANmodule->CANerrorStatus |= CO_CAN_ERRTX_OVERFLOW;
        }
        err = CO_ERROR_TX_OVERFLOW;
    }

    if (CANmodule->CANtxCount > 0U)
    {
        CO_CANtx_t *bufferTx = NULL;
        CO_CANtx_t *bufferTemp = &CANmodule->txArray[0];

        for (uint16_t i = CANmodule->txSize; i > 0U; i--)
        {
            if ((bufferTemp->bufferFull) || (bufferTemp == buffer))
            {
                bufferTx = bufferTemp;
                break;
            }

            bufferTemp++;
        }

        if (buffer != bufferTx)
        {

            if (!buffer->bufferFull)
            {
                buffer->bufferFull = true;
                CANmodule->CANtxCount++;
            }

            buffer = bufferTx;
        }
    }

    if (!prv_send_can_message(CANmodule, buffer))
    {
        if (buffer->bufferFull)
        {
            buffer->bufferFull = false;
            if (CANmodule->CANtxCount > 0U)
            {
                CANmodule->CANtxCount--;
            }
            CANmodule->bufferInhibitFlag = buffer->syncFlag;
        }
    }
    else
    {
        if (!buffer->bufferFull)
        {
            // If no buffer is free, message will be sent by interrupt
            buffer->bufferFull = true;
            CANmodule->CANtxCount++;
        }
    }

    CO_UNLOCK_CAN_SEND(CANmodule);

    return err;
}


void CO_CANclearPendingSyncPDOs(CO_CANmodule_t *CANmodule)
{
    uint32_t tpdoDeleted = 0U;

    // Send message to CAN network, Lock interrupts for atomic operation
    CO_LOCK_CAN_SEND(CANmodule);

    /* Abort message from CAN module, if there is synchronous TPDO.
     * Take special care with this functionality. */
    if (/*messageIsOnCanBuffer && */ CANmodule->bufferInhibitFlag)
    {
        // Clear TXREQ
        CANmodule->bufferInhibitFlag = false;
        tpdoDeleted = 1U;
    }

    // Delete also pending synchronous TPDOs in TX buffers
    if (CANmodule->CANtxCount > 0U)
    {
        for (uint16_t i = CANmodule->txSize; i > 0U; i--)
        {
            CO_CANtx_t *buffer = &CANmodule->txArray[i];

            if (buffer->bufferFull)
            {
                if (buffer->syncFlag)
                {
                    buffer->bufferFull = false;
                    if (CANmodule->CANtxCount > 0U)
                    {
                        CANmodule->CANtxCount--;
                    }
                    tpdoDeleted = 2U;
                }
            }
        }
    }

    if (tpdoDeleted)
    {
        CANmodule->CANerrorStatus |= CO_CAN_ERRTX_PDO_LATE;
    }

    CO_UNLOCK_CAN_SEND(CANmodule);
}


void CO_CANmodule_process(CO_CANmodule_t *CANmodule)
{
    uint32_t err = 0U;

    err = ((uint32_t)txErrors << 16) | ((uint32_t)rxErrors << 8) | overflow;

    if (CANmodule->errOld != err)
    {
        uint16_t status = CANmodule->CANerrorStatus;

        CANmodule->errOld = err;

        if (txErrors >= 256U)
        {
            // Bus off
            status |= CO_CAN_ERRTX_BUS_OFF;
        }
        else
        {
            // Recalculate CANerrorStatus, first clear some flags
            status &= 0xFFFFU ^ (CO_CAN_ERRTX_BUS_OFF |
                                 CO_CAN_ERRRX_WARNING | CO_CAN_ERRRX_PASSIVE |
                                 CO_CAN_ERRTX_WARNING | CO_CAN_ERRTX_PASSIVE);

            // Tx bus warning or passive
            if (rxErrors >= 128)
            {
                status |= CO_CAN_ERRRX_WARNING | CO_CAN_ERRRX_PASSIVE;
            }
            else if (rxErrors >= 96)
            {
                status |= CO_CAN_ERRRX_WARNING;
            }

            // Tx bus warning or passive
            if (txErrors >= 128)
            {
                status |= CO_CAN_ERRTX_WARNING | CO_CAN_ERRTX_PASSIVE;
            }
            else if (rxErrors >= 96)
            {
                status |= CO_CAN_ERRTX_WARNING;
            }

            // If not tx passive clear also overflow
            if ((status & CO_CAN_ERRTX_PASSIVE) == 0)
            {
                status &= 0xFFFFU ^ CO_CAN_ERRTX_OVERFLOW;
            }
        }

        if (overflow != 0U)
        {
            // CAN RX bus overflow
            status |= CO_CAN_ERRRX_OVERFLOW;
        }

        CANmodule->CANerrorStatus = status;
    }
}
