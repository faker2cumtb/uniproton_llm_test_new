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

#ifndef RESOURCE_TABLE_H__
#define RESOURCE_TABLE_H__

#include <stddef.h>
#include <openamp/rpmsg.h>
#include <openamp/remoteproc.h>
#include "openamp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VDEV_ID                 0xFF
#define VRING0_ID 0 /* (master to remote) fixed to 0 for Linux compatibility */
#define VRING1_ID 1 /* (remote to master) fixed to 1 for Linux compatibility */

#define RPMSG_IPU_C0_FEATURES   1
#define NUM_RPMSG_BUFF          8

#define VIRTIO_ID_RPMSG         7
#define RSC_VENDOR_EPT_TABLE    128 /* List of bound endpoints */

#define CPU_ON_FUNCID    0xC4000003
#define CPU_OFF_FUNCID   0x84000002
#define SYSTEM_RESET     0x84000009

#define POWERDOWN_PROT_FUNCID    0xFF000001

#define RSC_TABLE_SIZE 	   0x1000

enum rsc_table_entries {
    RSC_TABLE_EPT_TABLE_ENTRY,
#ifdef OS_GDB_STUB
    RSC_TABLE_RBUF_ENTRY,
#endif
#ifdef OS_OPTION_LOG
    RSC_TABLE_LOGBUF_ENTRY,
#endif
#ifdef OS_OPTION_POWER_DOWN_PROTECT
    RSC_TABLE_POWEROFF_PROTECT_ENTRY,
#endif
    RSC_TABLE_VDEV_ENTRY,
    RSC_TABLE_NUM_ENTRY
};

METAL_PACKED_BEGIN
struct ept_info {
    char name[RPMSG_NAME_SIZE];
    uint32_t addr;
    uint32_t dest_addr;
} METAL_PACKED_END;

#define MAX_NUM_OF_EPTS 64

METAL_PACKED_BEGIN
struct fw_rsc_ept {
    uint32_t type;
    uint32_t num_of_epts;
    struct ept_info endpoints[MAX_NUM_OF_EPTS];
} METAL_PACKED_END;

#ifdef OS_GDB_STUB
#define RSC_VENDOR_RINGBUFFER   129
#define RINGBUFFER_TOTAL_SIZE   0x2000

METAL_PACKED_BEGIN
struct fw_rsc_rbuf_pair {
	uint32_t type;
	uint32_t flags;
	uint64_t da;
	uint64_t pa;
	uint64_t len;
	uint8_t state;
	uint8_t reserved[7];
} METAL_PACKED_END;

enum rbuf_state {
	RBUF_STATE_UNINIT = 0,
	RBUF_STATE_INIT = 1,
	RBUF_STATE_ORDINARY_DATA = 2,
	RBUF_STATE_CTRL_C = 3,
	RBUF_STATE_RESTART = 4,
};

extern uint8_t get_rbuf_state(void);

#endif

#ifdef OS_OPTION_LOG
#define RSC_VENDOR_LOGBUFFER   130
METAL_PACKED_BEGIN
struct fw_rsc_log_buf {
    uint32_t type;
    uint32_t flags;
    uint64_t da;
    uint64_t pa;
    uint64_t len;
    uint8_t state;
    uint8_t reserved[7];
} METAL_PACKED_END;

enum logbuf_state {
    LOGBUF_STATE_UNINIT = 0,
    LOGBUF_STATE_INIT = 1,
};
#endif

#ifdef OS_OPTION_POWER_DOWN_PROTECT
#define RSC_VENDOR_POWEROFF_PROTECT_BUFFER   131
METAL_PACKED_BEGIN
struct fw_rsc_powerdown_protect_buf {
    uint32_t type;
    uint32_t flags;
    uint64_t da;
    uint64_t pa;
    uint64_t len;
} METAL_PACKED_END;
#endif

METAL_PACKED_BEGIN
struct fw_resource_table {
    unsigned int ver;
    unsigned int num;
    unsigned int reserved[2];
    unsigned int offset[RSC_TABLE_NUM_ENTRY];

    struct fw_rsc_ept ept_table;
#ifdef OS_GDB_STUB
    struct fw_rsc_rbuf_pair rbufs;
#endif
#ifdef OS_OPTION_LOG
    struct fw_rsc_log_buf logbuf;
#endif
#ifdef OS_OPTION_POWER_DOWN_PROTECT
    struct fw_rsc_powerdown_protect_buf powerdown_protect_buf;
#endif
    struct fw_rsc_vdev vdev;
    struct fw_rsc_vdev_vring vring0;
    struct fw_rsc_vdev_vring vring1;

} METAL_PACKED_END;


void rsc_table_get(void **table_ptr, int *length);
void rsc_table_set_offline_flag(void);

#ifdef __cplusplus
}
#endif

#endif
