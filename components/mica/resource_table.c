/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-04-08
 * Description: This file populates resource table for UniProton, for use by the Linux host.
 */

#include "resource_table.h"
#include "cpu_config.h"

#ifndef OS_SEC_RSC_TABLE
#define OS_SEC_RSC_TABLE __attribute__((keep, section(".resource_table")))
#endif

OS_SEC_RSC_TABLE static struct fw_resource_table resource_table = {
    .ver = 1,
    .num = RSC_TABLE_NUM_ENTRY,
    .offset = {
        offsetof(struct fw_resource_table, ept_table),
#ifdef OS_GDB_STUB
        offsetof(struct fw_resource_table, rbufs),
#endif
#ifdef OS_OPTION_LOG
        offsetof(struct fw_resource_table, logbuf),
#endif
#ifdef OS_OPTION_POWER_DOWN_PROTECT
        offsetof(struct fw_resource_table, powerdown_protect_buf),
#endif
        offsetof(struct fw_resource_table, vdev),
    },

    .ept_table = {
        .type = RSC_VENDOR_EPT_TABLE,
	.num_of_epts = 0,
    },
#ifdef OS_GDB_STUB
    .rbufs = {RSC_VENDOR_RINGBUFFER, 0, 0, 0, RINGBUFFER_TOTAL_SIZE, 0, {0}},
#endif
#ifdef OS_OPTION_LOG
    .logbuf = {RSC_VENDOR_LOGBUFFER, 0, SHM_LOG_BASE_ADDR, 0, LOGBUFFER_TOTAL_SIZE, 0, {0}},
#endif
#ifdef OS_OPTION_POWER_DOWN_PROTECT
    .powerdown_protect_buf = {RSC_VENDOR_POWEROFF_PROTECT_BUFFER, 0, 0, 0, 
                POWEROFF_PROTECT_BUFFER_TOTAL_SIZE},
#endif
    /* Virtio device entry */
    .vdev = {
        RSC_VDEV, VIRTIO_ID_RPMSG, 2, RPMSG_IPU_C0_FEATURES, 0, 0, 0,
        VRING_COUNT, {0, 0},
    },

    /* Vring rsc entry - part of vdev rsc entry */
    .vring0 = {VRING_TX_ADDRESS, VRING_ALIGNMENT,
                   NUM_RPMSG_BUFF, VRING0_ID, 0},
    .vring1 = {VRING_RX_ADDRESS, VRING_ALIGNMENT,
           NUM_RPMSG_BUFF, VRING1_ID, 0},
};

void rsc_table_get(void **table_ptr, int *length)
{
    *table_ptr = (void *)(MMU_OPENAMP_ADDR);
    *length = RSC_TABLE_SIZE;
}

#define PA2VA(pa) (pa)

#ifdef OS_GDB_STUB
#include "prt_gdbstub_ext.h"
static STUB_DATA struct GdbRingBufferCfg g_rbufCfg;
STUB_TEXT struct GdbRingBufferCfg *OsGetGdbRingBufferCfg()
{
    void *rsc;
    int rsc_size;
    struct fw_resource_table *rsc_table;

    rsc_table_get(&rsc, &rsc_size);
    rsc_table = (struct fw_resource_table *)rsc;

    uintptr_t tx_va, rx_va;

    while (rsc_table->rbufs.state == RBUF_STATE_UNINIT) {
        os_asm_invalidate_dcache_all();
    }

    os_asm_invalidate_dcache_all();
    tx_va = PA2VA(rsc_table->rbufs.pa);
    rx_va = tx_va + RINGBUFFER_TOTAL_SIZE / 2;
    g_rbufCfg.rxaddr = rx_va;
    g_rbufCfg.txaddr = tx_va;
    g_rbufCfg.size = RINGBUFFER_TOTAL_SIZE / 2;
    return &g_rbufCfg;
}
#endif

#ifdef OS_OPTION_LOG
uintptr_t OsGetLogBufferCfg()
{
    void *rsc;
    int rsc_size;
    struct fw_resource_table *rsc_table;

    rsc_table_get(&rsc, &rsc_size);
    rsc_table = (struct fw_resource_table *)rsc;

    return PA2VA(rsc_table->logbuf.da);
}

void setLogBufInitState()
{
    void *rsc;
    int rsc_size;
    struct fw_resource_table *rsc_table;

    rsc_table_get(&rsc, &rsc_size);
    rsc_table = (struct fw_resource_table *)rsc;
    rsc_table->logbuf.state = LOGBUF_STATE_INIT;
}

#endif

#ifdef OS_OPTION_POWER_DOWN_PROTECT
uintptr_t OsGetPowerDownProtBuffer()
{
    void *rsc;
    int rsc_size;
    struct fw_resource_table *rsc_table;

    rsc_table_get(&rsc, &rsc_size);
    rsc_table = (struct fw_resource_table *)rsc;

    return PA2VA(rsc_table->powerdown_protect_buf.da);
}
#endif

/* 下线时，设置rsc_table的reserved[1]为CPU_OFF_FUNCID，告诉mica侧更新状态 */
void rsc_table_set_offline_flag(void)
{
    void *rsc;
    int rsc_size;
    struct fw_resource_table *rsc_table;

    rsc_table_get(&rsc, &rsc_size);
    rsc_table = (struct fw_resource_table *)rsc;

    rsc_table->reserved[0] = CPU_OFF_FUNCID;
    os_asm_invalidate_dcache_all();
}