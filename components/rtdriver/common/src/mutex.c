/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-25     LuoYuncong   the first version
 */
#include <rtapi.h>

rt_err_t rt_mutex_init(rt_mutex_t mutex, const char *name, rt_uint8_t flag)
{
    if (PRT_SemMutexCreate(&mutex->sem) == OS_OK)
    {
        return RT_EOK;
    }

    return -RT_ERROR;
}

rt_err_t rt_mutex_detach(rt_mutex_t mutex)
{
    if (PRT_SemDelete(mutex->sem) == OS_OK)
    {
        return RT_EOK;
    }

    return -RT_ERROR;
}

rt_err_t rt_mutex_take(rt_mutex_t mutex, rt_int32_t timeout)
{
    U32 tick = timeout;

    if (timeout == RT_WAITING_FOREVER)
    {
        tick = OS_WAIT_FOREVER;
    }

    if (PRT_SemPend(mutex->sem, tick) == OS_OK)
    {
        return RT_EOK;
    }

    return -RT_ETIMEOUT;
}

rt_err_t rt_mutex_release(rt_mutex_t mutex)
{
    if (PRT_SemPost(mutex->sem) == OS_OK)
    {
        return RT_EOK;
    }

    return -RT_ERROR;
}
