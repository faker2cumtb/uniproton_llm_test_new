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
 * Create: 2024-04-15
 * Description: This file populates resource table for UniProton, for use by the Linux host.
 */

#include "prt_typedef.h"
#include "prt_hwi.h"
#include "prt_sys.h"
#include "rpmsg_backend.h"
#include "resource_table.h"
#include "rpmsg_service.h"
#include "print.h"


static int mica_init_flag = 0;

static void mica_ipi_handler(uintptr_t HwiArg)
{
    (void)HwiArg;
    void *rsc;
    int rsc_size;
    uint32_t status;
    struct fw_resource_table *rsc_table;

    rsc_table_get(&rsc, &rsc_size);
    rsc_table = (struct fw_resource_table *)rsc;

    //os_asm_invalidate_dcache_all();
    status = rsc_table->reserved[0];
#ifdef OS_GDB_STUB
#define NOTIFY_VAL 2
    if (rsc_table->rbufs.state == RBUF_STATE_CTRL_C) {
        rsc_table->rbufs.state = RBUF_STATE_ORDINARY_DATA;
        os_asm_invalidate_dcache_all();
        OsNotifyDie(NOTIFY_VAL, NULL);
    }
#endif
    if (status == CPU_ON_FUNCID || status == 0) {
        /* normal work */
        active_rpmsg_task();
    } else if (status == SYSTEM_RESET) {
        /* attach work: reset virtqueue */
        reset_vq();
        /* clear reserved[0] as the reset work is done */
        rsc_table->reserved[0] = 0;
        //os_asm_invalidate_dcache_all();
    } else if (status == CPU_OFF_FUNCID) {
#if defined(OS_OPTION_POWEROFF)
#if defined(OS_GDB_STUB)
        rsc_table->rbufs.state = RBUF_STATE_RESTART;
        os_asm_invalidate_dcache_all();
#endif
        PRT_SysPowerOff();
#endif
    }
#if defined(OS_OPTION_POWER_DOWN_PROTECT)
    else if (status == POWERDOWN_PROT_FUNCID) {
        notify_powerdown_msg();
    }
#endif
}

U32 mica_hwi_init(void)
{
    U32 ret;

    ret = PRT_HwiSetAttr(OS_OPENAMP_NOTIFY_HWI_NUM, OS_OPENAMP_NOTIFY_HWI_PRIO, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(OS_OPENAMP_NOTIFY_HWI_NUM, (HwiProcFunc)mica_ipi_handler, 0);
    if (ret != OS_OK) {
        return ret;
    }

#if (OS_GIC_VER == 3)
    ret = PRT_HwiEnable(OS_OPENAMP_NOTIFY_HWI_NUM);
    if (ret != OS_OK) {
        return ret;
    }
#endif

    return OS_OK;
}

int mica_service_init(void)
{
    int ret;

    if(mica_init_flag)
    {
        PRT_Printf("[mica] mica already init\n");
        return OS_OK;
    }

    ret = mica_hwi_init();
    if (ret) {
        PRT_Printf("[mica] mica_backend_init %d\n", ret);
        return OS_ERROR;
    }

    /* init rpmsg device */
    ret = create_rpmsg_device();
    if (ret) {
        PRT_Printf("[mica] create_rpmsg_device failed %d\n", ret);
        goto clean_mica_hwi;
    }

    ret = create_rpmsg_listen_task();
    if(ret)
    {
        PRT_Printf("[mica] create_rpmsg_listen_task failed %d\n", ret);
        goto clean_rpmsg_device;
    }

    ret = create_rpc_ept();
    if (ret) {
        PRT_Printf("[mica] create_rpc_ept failed %d\n", ret);
        goto clean_rpmsg_listen_task;
    }

    ret = create_tty_ept();
    if (ret) {
        PRT_Printf("[mica] create_tty_ept failed %d\n", ret);
        goto clean_rpc_ept;
    }

    OsSetOfflineFlagHook(rsc_table_set_offline_flag);

    mica_init_flag = 1;
    return OS_OK;

clean_rpc_ept:
    remove_rpc_ept();
clean_rpmsg_listen_task:
    remove_rpmsg_listen_task();
clean_rpmsg_device:
    rpmsg_backend_remove();
clean_mica_hwi:
    PRT_HwiDelete(OS_OPENAMP_NOTIFY_HWI_NUM);

    return OS_ERROR;
}

void mica_service_uninit(void)
{
    remove_tty_ept();
    remove_rpc_ept();
    remove_rpmsg_listen_task();
    rpmsg_backend_remove();
    PRT_HwiDelete(OS_OPENAMP_NOTIFY_HWI_NUM);

    mica_init_flag = 0;
}