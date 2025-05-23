/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-11-20
 * Description: openamp configuration
 */

#include "rpmsg_backend.h"
#include "prt_sem.h"
#include "prt_proxy_ext.h"
#include "pthread.h"
#include "stdio.h"
#include "prt_atomic.h"
#include "prt_config.h"
#include "print.h"

static SemHandle msg_sem;
static pthread_t listen_thread = NULL;

static void *rpmsg_listen_task(void *arg)
{
    /* Waiting for messages from host */
    while (1) {
        U32 ret = PRT_SemPend(msg_sem, OS_WAIT_FOREVER);
        if (ret == OS_OK)
            deal_rpmsg_msg();
        }
    return NULL;
}

void active_rpmsg_task()
{
    PRT_SemPost(msg_sem);
}

int create_rpmsg_listen_task()
{
    int ret;

    ret = PRT_SemCreate(0, &msg_sem);
    if (ret) {
        PRT_Printf("[rpmsg] SemCreate failed %d\n", ret);
        return OS_ERROR;
    }

    ret = pthread_create(&listen_thread, NULL, rpmsg_listen_task, NULL);
    if (ret != 0) {
        /* If no rpmsg tasks, release the backend. */
        PRT_SemDelete(msg_sem);
        PRT_Printf("[rpmsg] create rpmsg_listen_task fail, %d\n", ret);
        return -1;
    }

    return 0;
}

void remove_rpmsg_listen_task()
{
    if(listen_thread)
    {
        pthread_cancel(listen_thread);
        listen_thread = NULL;
    }
    PRT_SemDelete(msg_sem);
}