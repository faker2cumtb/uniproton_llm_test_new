/**
 *******************************************************************************
 *
 * @file CO_app_target.c
 * @author shianhu@ncti-gba.cn
 * @brief CANopen main program file for TARGET.
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

#include "CO_app_target.h"

#include <stdio.h>

#include "prt_sys.h"
#include "prt_clk.h"

#include "CANopen.h"
#include "OD.h"

// Default values for CO_CANopenInit()
#define OD_STATUS_BITS       NULL
#define NMT_CONTROL (CO_NMT_STARTUP_TO_OPERATIONAL | \
                     CO_NMT_ERR_ON_ERR_REG | \
                     CO_ERR_REG_GENERIC_ERR | \
                     CO_ERR_REG_COMMUNICATION)
#define FIRST_HB_TIME        500
#define SDO_SRV_TIMEOUT_TIME 1000
#define SDO_CLI_TIMEOUT_TIME 500
#define SDO_CLI_BLOCK        false


// Global pointer to the instance of CANopen object
static CO_t *g_CO_p = NULL;
// Global pointer to the instance of CANopenNodeRt object
static CANopenNodeRt *g_CANopenNodeRt_p = NULL;

static uint32_t time_previous_ms, time_current_ms;
static CO_ReturnError_t co_err;


static uint32_t HAL_GetTickMs()
{
    return (uint32_t)PRT_ClkCycle2Ms(PRT_ClkGetCycleCount64());
}


static void HAL_SystemReset()
{
    PRT_SysReboot();
}


/* This function will reset the CAN communication peripheral and
 * also the CANOpen stack variables */
static int canopen_app_resetCommunication()
{
    // CANopen communication reset - initialize CANopen objects
    PRT_Printf("CANopenNode - Reset communication ...\n");

    g_CO_p->CANmodule->CANnormal = false;

    // Enter CAN configuration
    CO_CANsetConfigurationMode((void*)g_CANopenNodeRt_p);
    CO_CANmodule_disable(g_CO_p->CANmodule);

    // Initialize CANopen
    co_err = CO_CANinit(g_CO_p, g_CANopenNodeRt_p, 0U);
    if (co_err != CO_ERROR_NO)
    {
        PRT_Printf("ERROR: CO_CANinit() failed, co_err=%d\n", co_err);
        return 1;
    }

    CO_LSS_address_t lssAddress =
    {
        .identity = {
            .vendorID = OD_PERSIST_COMM.x1018_identity.vendor_ID,
            .productCode = OD_PERSIST_COMM.x1018_identity.productCode,
            .revisionNumber = OD_PERSIST_COMM.x1018_identity.revisionNumber,
            .serialNumber = OD_PERSIST_COMM.x1018_identity.serialNumber
        }
    };
    co_err = CO_LSSinit(g_CO_p,
                        &lssAddress,
                        &g_CANopenNodeRt_p->desiredNodeID,
                        &g_CANopenNodeRt_p->canBaudRate);
    if (co_err != CO_ERROR_NO)
    {
        PRT_Printf("ERROR: CO_LSSinit() failed, co_err=%d\n", co_err);
        return 2;
    }

    g_CANopenNodeRt_p->activeNodeID = g_CANopenNodeRt_p->desiredNodeID;

    uint32_t errInfo = 0U;
    co_err = CO_CANopenInit(g_CO_p, ///< CANopen object
                            NULL, ///< alternate NMT
                            NULL, ///< alternate em
                            OD, ///< Object dictionary
                            OD_STATUS_BITS, ///< Optional OD_statusBits
                            NMT_CONTROL, ///< CO_NMT_control_t
                            FIRST_HB_TIME, ///< firstHBTime_ms
                            SDO_SRV_TIMEOUT_TIME, ///< SDOserverTimeoutTime_ms
                            SDO_CLI_TIMEOUT_TIME, ///< SDOclientTimeoutTime_ms
                            SDO_CLI_BLOCK, ///< SDOclientBlockTransfer
                            g_CANopenNodeRt_p->activeNodeID,
                            &errInfo);
    if (co_err != CO_ERROR_NO && co_err != CO_ERROR_NODE_ID_UNCONFIGURED_LSS)
    {
        if (co_err == CO_ERROR_OD_PARAMETERS)
        {
            PRT_Printf("ERROR: Object Dictionary entry 0x%X\n", errInfo);
        }
        else
        {
            PRT_Printf("ERROR: CO_CANopenInit() failed, co_err=%d\n", co_err);
        }
        return 3;
    }

    co_err = CO_CANopenInitPDO(g_CO_p,
                               g_CO_p->em,
                               OD,
                               g_CANopenNodeRt_p->activeNodeID,
                               &errInfo);
    if (co_err != CO_ERROR_NO)
    {
        if (co_err == CO_ERROR_OD_PARAMETERS)
        {
            PRT_Printf("ERROR: Object Dictionary entry 0x%X\n", errInfo);
        }
        else
        {
            PRT_Printf("ERROR: CO_CANopenInitPDO() failed, co_err=%d\n", co_err);
        }
        return 4;
    }

    // Configure Timer interrupt function for execution every 1 millisecond
    rt_hwtimerval_t tv = {0};
    tv.sec = 0;
    tv.usec = 1 * 1000; // 1 ms
    rt_size_t rt_rv = rt_device_write(g_CANopenNodeRt_p->timerHandle, 0, &tv, sizeof(tv));
    if (rt_rv != sizeof(tv))
    {
        PRT_Printf("ERROR: rt_device_write() failed, rt_rv=%d, size=%zu\n", rt_rv, sizeof(tv));
        return 5;
    }

    // Configure CAN transmit and receive interrupt if needed

    // Configure CANopen callbacks, etc
    if (!g_CO_p->nodeIdUnconfigured)
    {
#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
        if (storageInitError != 0U)
        {
            CO_errorReport(g_CO_p->em,
                           CO_EM_NON_VOLATILE_MEMORY,
                           CO_EMC_HARDWARE,
                           storageInitError);
        }
#endif
    }
    else
    {
        PRT_Printf("CANopenNode - Node-id not initialized\n");
    }

    // Start CAN module
    CO_CANsetNormalMode(g_CO_p->CANmodule);

    time_previous_ms = time_current_ms = HAL_GetTickMs();

    PRT_Printf("CANopenNode - Running ...\n");

    return 0;
}


int canopen_app_init(CANopenNodeRt *canopenNodeRt_p)
{
    // Keep a copy global reference of CANopenNodeRt object
    g_CANopenNodeRt_p = canopenNodeRt_p;

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
    CO_storage_t storage;
    CO_storage_entry_t storageEntries[] =
    {
        {
            .addr = &OD_PERSIST_COMM,
            .len = sizeof(OD_PERSIST_COMM),
            .subIndexOD = 2,
            .attr = CO_storage_cmd | CO_storage_restore,
            .addrNV = NULL
        }
    };
    uint8_t storageEntriesCount = (sizeof(storageEntries) / sizeof(storageEntries[0]));
    uint32_t storageInitError = 0U;
#endif

    // Allocate memory
    CO_config_t *config_p = NULL;

#ifdef CO_MULTIPLE_OD
    // Example usage of CO_MULTIPLE_OD (but still single OD here)
    CO_config_t co_config = {0};
    OD_INIT_CONFIG(co_config); ///< helper macro from OD.h
    co_config.CNT_LEDS = 1;
    co_config.CNT_LSS_SLV = 1;
    config_p = &co_config;
#endif

    uint32_t heapMemoryUsed;

    g_CO_p = CO_new(config_p, &heapMemoryUsed);
    if (g_CO_p == NULL)
    {
        PRT_Printf("ERROR: Can not allocate memory\n");
        return 1;
    }
    else
    {
        PRT_Printf("Allocated %u bytes for CANopen objects\n", heapMemoryUsed);
    }

    g_CO_p->CANmodule->CANptr = g_CANopenNodeRt_p;
    g_CANopenNodeRt_p->canOpenStack = g_CO_p;

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
    co_err = CO_storageBlank_init(&storage, g_CO_p->CANmodule,
                                  OD_ENTRY_H1010_storeParameters,
                                  OD_ENTRY_H1011_restoreDefaultParameters,
                                  storageEntries,
                                  storageEntriesCount,
                                  &storageInitError);
    if ((co_err != CO_ERROR_NO) && (co_err != CO_ERROR_DATA_CORRUPT))
    {
        PRT_Printf("ERROR: Storage %d\n", storageInitError);
        return 2;
    }
#endif

    canopen_app_resetCommunication();

    return 0;
}


void canopen_app_process()
{
    // Get time difference since last function call
    time_current_ms = HAL_GetTickMs();

    // Make sure more than 1ms elapsed
    if ((time_current_ms - time_previous_ms) > 0U)
    {
        uint32_t timeDifference_us = (time_current_ms - time_previous_ms) * 1000;
        time_previous_ms = time_current_ms;

        // CANopen process
        CO_NMT_reset_cmd_t reset_status = CO_process(g_CO_p,
                                                     false,
                                                     timeDifference_us,
                                                     NULL);

        g_CANopenNodeRt_p->outStatusLedRed = CO_LED_RED(g_CO_p->LEDs, CO_LED_CANopen);
        g_CANopenNodeRt_p->outStatusLedGreen = CO_LED_GREEN(g_CO_p->LEDs, CO_LED_CANopen);

        if (reset_status == CO_RESET_COMM)
        {
            PRT_Printf("CANopenNode Reset Communication request\n");

            rt_err_t rt_rc = rt_device_close(g_CANopenNodeRt_p->timerHandle);
            if (rt_rc != RT_EOK)
            {
                PRT_Printf("ERROR: rt_device_close() failed, rt_rc=%d\n", rt_rc);
            }

            CO_CANsetConfigurationMode((void*)g_CANopenNodeRt_p);

            // Delete objects from memory
            CO_delete(g_CO_p);

            // Reset Communication routine
            canopen_app_init(g_CANopenNodeRt_p);
        }
        else if (reset_status == CO_RESET_APP)
        {
            PRT_Printf("CANopenNode - Reset device ...\n");

            // Reset the system
            HAL_SystemReset();
        }
    }
}


void canopen_app_interrupt()
{
    CO_LOCK_OD(g_CO_p->CANmodule);

    if (!g_CO_p->nodeIdUnconfigured && g_CO_p->CANmodule->CANnormal)
    {
        // Get time difference since last function call if needed
        const uint32_t timeDifference_us = 1000; // 1ms

        bool_t syncWas = false;

#if (CO_CONFIG_SYNC) & CO_CONFIG_SYNC_ENABLE
        syncWas = CO_process_SYNC(g_CO_p, timeDifference_us, NULL);
#endif

#if (CO_CONFIG_PDO) & CO_CONFIG_RPDO_ENABLE
        CO_process_RPDO(g_CO_p, syncWas, timeDifference_us, NULL);
#endif

#if (CO_CONFIG_PDO) & CO_CONFIG_TPDO_ENABLE
        CO_process_TPDO(g_CO_p, syncWas, timeDifference_us, NULL);
#endif

#if (CO_CONFIG_SRDO) & CO_CONFIG_SRDO_ENABLE
        CO_process_SRDO(g_CO_p, timeDifference_us, NULL);
#endif

        // Further I/O or nonblocking application code may go here
    }

    CO_UNLOCK_OD(g_CO_p->CANmodule);
}
