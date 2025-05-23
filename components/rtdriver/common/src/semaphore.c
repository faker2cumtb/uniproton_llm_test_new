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

rt_err_t rt_sem_init(rt_sem_t sem, const char *name, rt_uint32_t value, rt_uint8_t flag)
{
    if (PRT_SemCreate(value, &sem->sem) == OS_OK)
    {
        return RT_EOK;
    }

    return -RT_ERROR;
}

rt_err_t rt_sem_detach(rt_sem_t sem)
{
    if (PRT_SemDelete(sem->sem) == OS_OK)
    {
        return RT_EOK;
    }

    return -RT_ERROR;
}

rt_err_t rt_sem_take(rt_sem_t sem, rt_int32_t timeout)
{
    U32 tick = timeout;

    if (timeout == RT_WAITING_FOREVER)
    {
        tick = OS_WAIT_FOREVER;
    }

    if (PRT_SemPend(sem->sem, tick) == OS_OK)
    {
        return RT_EOK;
    }

    return -RT_ETIMEOUT;
}

rt_err_t rt_sem_release(rt_sem_t sem)
{
    if (PRT_SemPost(sem->sem) == OS_OK)
    {
        return RT_EOK;
    }

    return -RT_ERROR;
}
