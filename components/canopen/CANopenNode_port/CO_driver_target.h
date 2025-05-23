/**
 *******************************************************************************
 *
 * @file CO_driver_target.h
 * @author shianhu@ncti-gba.cn
 * @brief Device and application specific definitions for CANopenNode.
 *        This file contains device and application specific definitions.
 *        It is included from CO_driver.h, which contains documentation
 *        for common definitions below.
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

#ifndef CO_DRIVER_TARGET_H
#define CO_DRIVER_TARGET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef STM32_CAN
    #include "cmsis_gcc.h"
#endif
#include "prt_hwi.h"
#include "print.h"
#include "rtdevice.h"

#ifdef CO_DRIVER_CUSTOM
    #include "CO_driver_custom.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Stack configuration override default values
 * For more information see file CO_config.h & CO_driver.h */

#undef CO_CONFIG_STORAGE_ENABLE // Do not need storage option right now

#define CO_CONFIG_NMT (CO_CONFIG_NMT_MASTER)
#define CO_CONFIG_TIME (CO_CONFIG_TIME_ENABLE | \
                        CO_CONFIG_TIME_PRODUCER | \
                        CO_CONFIG_GLOBAL_FLAG_CALLBACK_PRE | \
                        CO_CONFIG_GLOBAL_FLAG_OD_DYNAMIC)
#define CO_CONFIG_FIFO (CO_CONFIG_FIFO_ENABLE)
#define CO_CONFIG_SDO_CLI (CO_CONFIG_SDO_CLI_ENABLE)


// Basic definitions, OS specific/dependent
// If big endian, CO_SWAP_xx macros must swap bytes
#define CO_LITTLE_ENDIAN // or CO_BIG_ENDIAN
#define CO_SWAP_16(x) x
#define CO_SWAP_32(x) x
#define CO_SWAP_64(x) x
typedef uint_fast8_t bool_t;
typedef float float32_t;
typedef double float64_t;
// true and false are defined in stdbool.h
// NULL is defined in stddef.h
// int8_t to uint64_t are defined in stdint.h


// Basic definitions, microcontroller specific/dependent
// CAN masks for identifiers
#define CAN_ID_STD_MASK 0x7FFU      ///< CAN standard ID mask
#define FLAG_RTR        0x800U      ///< RTR flag, part of identifier
// Type for hardware interrupt number
typedef HwiHandle CO_HwInt_t;
// Type for CAN TX message
typedef struct rt_can_msg CO_CANtxMsg_t;
// Type for CAN RX message
typedef struct rt_can_msg CO_CANrxMsg_t;
// APIs to access the CAN RX message
#define CO_CANrxMsg_readIdent(msg) ((uint16_t)((CO_CANrxMsg_t*)(msg))->stdId)
#define CO_CANrxMsg_readDLC(msg)   ((uint8_t )((CO_CANrxMsg_t*)(msg))->len)
#define CO_CANrxMsg_readData(msg)  ((uint8_t*)((CO_CANrxMsg_t*)(msg))->data)

// Received message object
typedef struct CO_CANrx_S
{
    uint16_t ident;
    uint16_t mask;
    void *object;
    void (*CANrx_callback)(void *object, void *message);
    char *object_name;
    char *callback_name;
    char *func_name;
} CO_CANrx_t;

// Transmit message object
typedef struct CO_CANtx_S
{
    uint32_t ident;
    uint8_t DLC;
    uint8_t data[8];
    volatile bool_t bufferFull;
    volatile bool_t syncFlag;
    char *func_name;
} CO_CANtx_t;

// CAN module object
typedef struct CO_CANmodule_S
{
    void *CANptr;
    CO_CANrx_t *rxArray;
    uint16_t rxSize;
    CO_CANtx_t *txArray;
    uint16_t txSize;
    uint16_t CANerrorStatus;
    volatile bool_t CANnormal;
    volatile bool_t useCANrxFilters;
    volatile bool_t bufferInhibitFlag;
    volatile bool_t firstCANtxMessage;
    volatile uint16_t CANtxCount;
    uint32_t errOld;
    uintptr_t intSave_send; ///< register of interrupts for send operation
    uintptr_t intSave_emcy; ///< register of interrupts for emergency operation
    uintptr_t intSave_od; ///< register of interrupts for send operation
} CO_CANmodule_t;

// (un)lock critical section in CO_CANsend()
#define CO_LOCK_CAN_SEND(CAN_MODULE) (CAN_MODULE)->intSave_send = PRT_HwiLock()
#define CO_UNLOCK_CAN_SEND(CAN_MODULE) PRT_HwiRestore((CAN_MODULE)->intSave_send)

// (un)lock critical section in CO_errorReport() or CO_errorReset()
#define CO_LOCK_EMCY(CAN_MODULE) (CAN_MODULE)->intSave_emcy = PRT_HwiLock()
#define CO_UNLOCK_EMCY(CAN_MODULE) PRT_HwiRestore((CAN_MODULE)->intSave_emcy)

// (un)lock critical section when accessing Object Dictionary
#define CO_LOCK_OD(CAN_MODULE) (CAN_MODULE)->intSave_od = PRT_HwiLock()
#define CO_UNLOCK_OD(CAN_MODULE) PRT_HwiRestore((CAN_MODULE)->intSave_od)

// Synchronization between CAN receive and message processing threads
#define CO_MemoryBarrier() do { __sync_synchronize(); } while (0)
#define CO_FLAG_READ(rxNew) ((rxNew) != NULL)
#define CO_FLAG_SET(rxNew) \
    do { \
        CO_MemoryBarrier(); \
        rxNew = (void*)1L; \
    } while (0)
#define CO_FLAG_CLEAR(rxNew) \
    do { \
        CO_MemoryBarrier(); \
        rxNew = NULL; \
    } while (0)


void CO_CANopenRx_Isr();

void CO_CANopenTx_Isr();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CO_DRIVER_TARGET_H */
