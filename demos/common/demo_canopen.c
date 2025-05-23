/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-20     ShiAnhu      the first version
 */
#include <prt_hwi.h>
#include <prt_clk.h>
#include <prt_task.h>
#include <prt_config.h>
#include <prt_sys.h>
#include <hwi_router.h>
#include <print.h>
#include <CANopen.h>
#include <OD.h>
#include <CO_app_target.h>
#include <demo.h>

static CANopenNodeRt canopenNodeRt = {0};

static rt_err_t can_rx_indicate(rt_device_t can_dev, rt_size_t size)
{
    CO_CANopenRx_Isr();
    return RT_EOK;
}

static rt_err_t can_tx_indicate(rt_device_t can_dev, void *buffer)
{
    CO_CANopenTx_Isr();
    return RT_EOK;
}

static void HW_CAN_Init(const CO_CANhandle_t canHandle,
                        const uint16_t canBaudRate)
{
    if (canHandle == RT_NULL)
    {
        PRT_Printf("ERROR: CAN device is not found!\n");
        return;
    }

    rt_err_t rt_rc = RT_ERROR;

    /* 设置波特率，驱动支持10K, 20K, 50K, 100K, 125K, 250K, 500K, 800K, 1M */
    const uint64_t canBaudRate2 = canBaudRate * 1000;
    rt_rc = rt_device_control(canHandle, RT_CAN_CMD_SET_BAUD, (void*)canBaudRate2);
    if (rt_rc != RT_EOK)
    {
        PRT_Printf("ERROR: rt_device_control(SET_BAUD) failed, rt_rc=%d\n", rt_rc);
        return;
    }

    /* 设置CAN模式，驱动支持正常模式，监听模式 */
    rt_rc = rt_device_control(canHandle, RT_CAN_CMD_SET_MODE, (void*)RT_CAN_MODE_NORMAL);
    if (rt_rc != RT_EOK)
    {
        PRT_Printf("ERROR: rt_device_control(SET_MODE) failed, rt_rc=%d\n", rt_rc);
        return;
    }

    /* 过滤位设置，如果不设置过滤位，则无法接收数据 */
    /* E2000Q最多支持4个过滤器 */
    struct rt_can_filter_item items[] =
    {
        /* 只接收标准帧 */
        RT_CAN_FILTER_ITEM_INIT(0x00, RT_CAN_STDID, RT_CAN_DTR, 0, 0x00, RT_NULL, RT_NULL),
    };
    struct rt_can_filter_config cfg = {sizeof(items) / sizeof(items[0]), 1, items};

    rt_rc = rt_device_control(canHandle, RT_CAN_CMD_SET_FILTER, &cfg);
    if (rt_rc != RT_EOK)
    {
        PRT_Printf("ERROR: rt_device_control(SET_FILTER) failed, rt_rc=%d\n", rt_rc);
        return;
    }

    rt_rc = rt_device_set_rx_indicate(canHandle, can_rx_indicate);
    if (rt_rc != RT_EOK)
    {
        PRT_Printf("ERROR: rt_device_set_rx_indicate() failed, rt_rc=%d\n", rt_rc);
        return;
    }

    rt_rc = rt_device_set_tx_complete(canHandle, can_tx_indicate);
    if (rt_rc != RT_EOK)
    {
        PRT_Printf("ERROR: rt_device_set_tx_complete() failed, rt_rc=%d\n", rt_rc);
        return;
    }
}


static rt_err_t timeout_callback(rt_device_t timer_dev, rt_size_t size)
{
    rt_hwtimerval_t tv;

    rt_size_t rt_rv = rt_device_read(timer_dev, 0, &tv, sizeof(tv));
    if (rt_rv != sizeof(tv))
    {
        PRT_Printf("ERROR: rt_device_read() failed, rt_rv=%d, size=%d\n", rt_rv, sizeof(tv));
        return RT_ERROR;
    }

    PRT_Printf("timeout = %d.%06d s\n", tv.sec, tv.usec);

    static U64 clk_old = 0;

    U64 clk_now = PRT_ClkGetCycleCount64();

    // Check CO_CANopenTx for every 10ms
    if ((clk_now - clk_old) >= (OS_SYS_CLOCK / 100))
    {
        CO_CANopenTx_Isr();
    }

    canopen_app_interrupt();

    clk_old = clk_now;

    return 0;
}

static void HW_TIMER_Init(const CO_CANtimer_t timerHandle)
{
    if (timerHandle == RT_NULL)
    {
        PRT_Printf("ERROR: CAN device is not found!\n");
        return;
    }

    rt_err_t rt_rc = RT_ERROR;

    rt_rc = rt_device_open(timerHandle, RT_DEVICE_OFLAG_RDWR);
    if (rt_rc != RT_EOK)
    {
        PRT_Printf("ERROR: rt_device_open() failed, rt_rc=%d\n", rt_rc);
        return;
    }

    rt_rc = rt_device_set_rx_indicate(timerHandle, timeout_callback);
    if (rt_rc != RT_EOK)
    {
        PRT_Printf("ERROR: rt_device_set_rx_indicate() failed, rt_rc=%d\n", rt_rc);
        return;
    }

    rt_hwtimer_mode_t mode = HWTIMER_MODE_PERIOD;
    rt_rc = rt_device_control(timerHandle, HWTIMER_CTRL_MODE_SET, &mode);
    if (rt_rc != RT_EOK)
    {
        PRT_Printf("ERROR: rt_device_control(MODE_SET) failed, rt_rc=%d\n", rt_rc);
        return;
    }
}

static void CO_HW_Init(const CO_CANhandle_t canHandle,
                       const uint16_t canBaudRate,
                       const CO_CANtimer_t timerHandle)
{
    HW_CAN_Init(canHandle, canBaudRate);
    HW_TIMER_Init(timerHandle);
}


static inline void sleep_us(uint32_t time_us)
{
    PRT_ClkDelayUs(time_us);
}

static CO_SDO_abortCode_t read_SDO(CO_SDOclient_t *SDO_C, uint8_t nodeId,
                                   uint16_t index, uint8_t subIndex,
                                   uint8_t *buf, size_t bufSize, size_t *readSize)
{
    static uint8_t last_nodeId = 0;

    CO_SDO_return_t sdo_rc;

    if (last_nodeId != nodeId)
    {
        // Setup client
        sdo_rc = CO_SDOclient_setup(SDO_C,
                                    CO_CAN_ID_SDO_CLI + nodeId,
                                    CO_CAN_ID_SDO_SRV + nodeId,
                                    nodeId);
        if (sdo_rc != CO_SDO_RT_ok_communicationEnd)
        {
            PRT_Printf("ERROR in CO_SDOclient_setup(), sdo_rc=%d\n", sdo_rc);
            return CO_SDO_AB_GENERAL;
        }

        last_nodeId = nodeId;
    }

    // Initiate upload
    sdo_rc = CO_SDOclientUploadInitiate(SDO_C, index, subIndex, 1000, false);
    if (sdo_rc != CO_SDO_RT_ok_communicationEnd)
    {
        PRT_Printf("ERROR in CO_SDOclientUploadInitiate(), sdo_rc=%d\n", sdo_rc);
        return CO_SDO_AB_GENERAL;
    }

    // Upload data
    int32_t retry_max = 200;
    do
    {
        uint32_t timeDifference_us = 10000;
        CO_SDO_abortCode_t abortCode = CO_SDO_AB_NONE;

        sdo_rc = CO_SDOclientUpload(SDO_C,
                                    timeDifference_us,
                                    false,
                                    &abortCode,
                                    NULL, NULL, NULL);
        if (sdo_rc < 0)
        {
            PRT_Printf("ERROR in CO_SDOclientUpload(), sdo_rc=%d\n", sdo_rc);
            return abortCode;
        }

        sleep_us(timeDifference_us);
    } while ((sdo_rc > 0) && (--retry_max));

    if (!retry_max)
    {
        PRT_Printf("WARNING in CO_SDOclientUpload(), timeout\n");
        return CO_SDO_AB_TIMEOUT;
    }

    // Copy data to the user buffer (for long data function must be called
    // several times inside the loop)
    *readSize = CO_SDOclientUploadBufRead(SDO_C, buf, bufSize);

    return CO_SDO_AB_NONE;
}

static CO_SDO_abortCode_t write_SDO(CO_SDOclient_t *SDO_C, uint8_t nodeId,
                                    uint16_t index, uint8_t subIndex,
                                    uint8_t *data, size_t dataSize)
{
    static uint8_t last_nodeId = 0;

    CO_SDO_return_t sdo_rc;
    bool_t bufferPartial = false;

    if (last_nodeId != nodeId)
    {
        // Setup client
        sdo_rc = CO_SDOclient_setup(SDO_C,
                                    CO_CAN_ID_SDO_CLI + nodeId,
                                    CO_CAN_ID_SDO_SRV + nodeId,
                                    nodeId);
        if (sdo_rc != CO_SDO_RT_ok_communicationEnd)
        {
            PRT_Printf("ERROR in CO_SDOclient_setup(), sdo_rc=%d\n", sdo_rc);
            return -1;
        }

        last_nodeId = nodeId;
    }

    // Initiate download
    sdo_rc = CO_SDOclientDownloadInitiate(SDO_C, index, subIndex,
                                          dataSize, 1000, false);
    if (sdo_rc != CO_SDO_RT_ok_communicationEnd)
    {
        PRT_Printf("ERROR in CO_SDOclientDownloadInitiate(), sdo_rc=%d\n", sdo_rc);
        return -1;
    }

    // Fill data
    size_t nWritten = CO_SDOclientDownloadBufWrite(SDO_C, data, dataSize);
    if (nWritten < dataSize)
    {
        bufferPartial = true;
        // If SDO Fifo buffer is too small, data can be refilled in the loop
    }

    // Download data
    int32_t retry_max = 200;
    do
    {
        uint32_t timeDifference_us = 10000;
        CO_SDO_abortCode_t abortCode = CO_SDO_AB_NONE;

        sdo_rc = CO_SDOclientDownload(SDO_C,
                                      timeDifference_us,
                                      false,
                                      bufferPartial,
                                      &abortCode,
                                      NULL, NULL);
        if (sdo_rc < 0)
        {
            PRT_Printf("ERROR in CO_SDOclientDownload(), sdo_rc=%d\n", sdo_rc);
            return abortCode;
        }

        sleep_us(timeDifference_us);
    } while ((sdo_rc > 0) && (--retry_max));

    if (!retry_max)
    {
        PRT_Printf("WARNING in CO_SDOclientDownload(), timeout\n");
        return CO_SDO_AB_TIMEOUT;
    }

    return CO_SDO_AB_NONE;
}


void canopen_demo()
{
    PRT_Printf("Hello world! - IDE: Visual Studio Code, OS: UniProton, "
               "Built: %s %s\n", __DATE__, __TIME__);

#define LOCAL_ID 8
#define REMOTE_ID ((LOCAL_ID != 7) ? 7 : 8)
#define LOOP_TIME_MS 10
#define LOOP_PER_SEC (1000 / LOOP_TIME_MS)
#define LOOP_COUNT ((LOOP_TIME_MS <= 1000) ? (LOOP_PER_SEC * 10) : 10)
    CANopenNodeRt tCANopenNodeRt =
    {
        .desiredNodeID = LOCAL_ID,
        .canHandle = rt_device_find("can0"),
        .canBaudRate = CAN1MBaud / 1000,
        .timerHandle = rt_device_find("timer1"),
        .hwInitFunction = CO_HW_Init,
        .outStatusLedGreen = 0,
        .outStatusLedRed = 0,
        .canOpenStack = NULL,
    };
    canopenNodeRt = tCANopenNodeRt;
    canopen_app_init(&canopenNodeRt);
    int count = 0;
    do
    {
        count++;
#if LOCAL_ID == 8
#define _CO_OBJ canopenNodeRt
#define _HEX_FMT_ "%X"
#else
#define _CO_OBJ canOpenNodeSTM32
#define _HEX_FMT_ "%lX"
#endif
        OD_PERSIST_COMM.x6002_txPdo = (0x20000000U | (LOCAL_ID << 24)) + count;
        OD_PERSIST_COMM.x6004_txSdoSrv = (0x40000000U | (LOCAL_ID << 24)) + count;
        OD_PERSIST_COMM.x6006_txSdoCli = (0x60000000U | (LOCAL_ID << 24)) + count;

        if ((count % LOOP_COUNT) == (LOOP_PER_SEC * 1))
        {
            PRT_Printf("count=%ld\n", count);
        }

        if (1 && ((count % LOOP_COUNT) == (LOOP_PER_SEC * 3)))
        {
            // RPDO
            PRT_Printf("INFO: x6001_rxPdo=0x" _HEX_FMT_ ", x6002_txPdo=0x" _HEX_FMT_ ", "
                       "x6003_rxSdoSrv=0x" _HEX_FMT_ ", x6004_txSdoSrv=0x" _HEX_FMT_ ", "
                       "x6005_rxSdoCli=0x" _HEX_FMT_ ", x6006_txSdoCli=0x" _HEX_FMT_ "\n",
                       OD_PERSIST_COMM.x6001_rxPdo, OD_PERSIST_COMM.x6002_txPdo,
                       OD_PERSIST_COMM.x6003_rxSdoSrv, OD_PERSIST_COMM.x6004_txSdoSrv,
                       OD_PERSIST_COMM.x6005_rxSdoCli, OD_PERSIST_COMM.x6006_txSdoCli);
            // TPDO
            CO_TPDOsendRequest(&_CO_OBJ.canOpenStack->TPDO[0]);
        }

        if (1 && ((count % LOOP_COUNT) == (LOOP_PER_SEC * 5)))
        {
            uint32_t data_rx = 0;
            size_t size_rx = 0;
            CO_SDO_abortCode_t sdo_rc = CO_SDO_AB_NONE;
#if LOCAL_ID == 8
            sdo_rc = read_SDO(_CO_OBJ.canOpenStack->SDOclient, REMOTE_ID, 0x6006U, 0,
                              (uint8_t*)&data_rx, sizeof(data_rx), &size_rx);
#else
            sdo_rc = read_SDO(_CO_OBJ.canOpenStack->SDOclient, REMOTE_ID, 0x6004U, 0,
                              (uint8_t*)&data_rx, sizeof(data_rx), &size_rx);
#endif
            if (sdo_rc != CO_SDO_AB_NONE)
            {
                if (sdo_rc != CO_SDO_AB_TIMEOUT)
                {
                    PRT_Printf("ERROR in read_SDO(), sdo_rc=0x%X\n", sdo_rc);
                }
            }
            else
            {
                PRT_Printf("INFO: read_SDO(data_rx=0x" _HEX_FMT_ ", size_rx=%u) OK\n", data_rx, size_rx);
#if LOCAL_ID == 8
                OD_PERSIST_COMM.x6005_rxSdoCli = data_rx;
#else
                OD_PERSIST_COMM.x6003_rxSdoSrv = data_rx;
#endif
            }
        }

        if (1 && ((count % LOOP_COUNT) == (LOOP_PER_SEC * 7)))
        {
            CO_SDO_abortCode_t sdo_rc = CO_SDO_AB_NONE;
#if LOCAL_ID == 8
            sdo_rc = write_SDO(_CO_OBJ.canOpenStack->SDOclient, REMOTE_ID, 0x6005U, 0,
                               (uint8_t*)&OD_PERSIST_COMM.x6006_txSdoCli, sizeof(OD_PERSIST_COMM.x6006_txSdoCli));
#else
            sdo_rc = write_SDO(_CO_OBJ.canOpenStack->SDOclient, REMOTE_ID, 0x6003U, 0,
                               (uint8_t*)&OD_PERSIST_COMM.x6004_txSdoSrv, sizeof(OD_PERSIST_COMM.x6004_txSdoSrv));
#endif
            if (sdo_rc != CO_SDO_AB_NONE)
            {
                if (sdo_rc != CO_SDO_AB_TIMEOUT)
                {
                    PRT_Printf("ERROR in write_SDO(), sdo_rc=0x%X\n", sdo_rc);
                }
            }
            else
            {
                PRT_Printf("INFO: write_SDO() OK\n");
            }
        }

        canopen_app_process();
        PRT_TaskDelay(LOOP_TIME_MS);
    } while (true);
}
