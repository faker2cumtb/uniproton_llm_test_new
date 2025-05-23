/**
 *******************************************************************************
 *
 * @file CO_app_target.h
 * @author shianhu@ncti-gba.cn
 * @brief Definition of CANopen for TARGET.
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

#ifndef CO_APP_TARGET_H_
#define CO_APP_TARGET_H_

#include "CANopen.h"
#include "rtdevice.h"

#ifdef __cplusplus
extern "C" {
#endif

// Type for CAN handle
typedef rt_device_t CO_CANhandle_t;

// Type for timer handle
typedef rt_device_t CO_CANtimer_t;

// Type for CAN RX message
//typedef struct CANFD_MSG CO_CANrxMsg_t;

/**
 * @brief Struct of CANopen Node for TARGET
 *
 * @param desiredNodeID  This is the node ID that you ask the CANOpen stack to
 *                       assign to your device, although it might not always be
 *                       the final node ID, after calling canopen_app_init()
 *                       you should check @ref activeNodeID for assigned node ID
 * @param activeNodeID   assigned node ID
 * @param canHandle      Pass in the CAN Handle to this function and it will be
 *                       used for all CAN Communications. It can be FDCan or CAN
 *                       and CANOpen Driver will take of care of handling that
 * @param canHwInt       CAN interrupt number
 * @param canBaudRate    CAN baud rate in unit of 1kBit/sec
 * @param timerHandle    Pass in the timer that is going to be used for
 *                       generating 1ms interrupt for timer thread function
 * @param timerHwInt     TIMER interrupt number
 * @param hwInitFunction Pass in the function that initialize the CAN peripheral
 * @param outStatusLedGreen This will be updated by the stack, use them for the
 *                          LED management
 * @param outStatusLedRed   This will be updated by the stack, use them for the
 *                          LED management
 * @param canOpenStack      CANopen stack object
 */
typedef struct CANopenNodeRt_S
{
    uint8_t desiredNodeID;
    uint8_t activeNodeID;
    CO_CANhandle_t canHandle;
    uint16_t canBaudRate; // Note: unit is 1kBit/sec
    CO_CANtimer_t timerHandle;
    void (*hwInitFunction)(const CO_CANhandle_t canHandle,
                           const uint16_t canBaudRate,
                           const CO_CANtimer_t timerHandle);
    uint8_t outStatusLedGreen;
    uint8_t outStatusLedRed;
    CO_t *canOpenStack;
} CANopenNodeRt;


/**
 * @brief This function will basically setup the CANopen node,
 *        it will initialize the required CANOpen Stack objects,
 *        allocate the memory and prepare stack for communication reset.
 *
 * @param canopenNodeRt_p pointer to CANopenNodeRt
 *
 * @return int
 */
int canopen_app_init(CANopenNodeRt *canopenNodeRt_p);


/**
 * @brief This function is the main loop for normal program execution,
 *        it will check the input buffers and any outstanding tasks
 *        that are not time critical, this function should be called
 *        regularly from your code (i.e from your while(1))
 */
void canopen_app_process();


/**
 * @brief Thread function executes in constant intervals,
 *        this function can be called from RTOS tasks or Timers
 */
void canopen_app_interrupt();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CO_APP_TARGET_H_ */
