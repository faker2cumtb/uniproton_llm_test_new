/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-09-30     Bernard      first version.
 * 2021-08-18     chenyingchun add comments
 * 2024-07-18     LuoYuncong   Port to UniProton
 * 2024-11-07     LuoYuncong   二值信号量可能有问题，改用普通信号量模拟完成量
 */

#include <rtapi.h>
#include <rtdevice.h>

/**
 * @brief This function will initialize a completion object.
 *
 * @param completion is a pointer to a completion object.
 */
void rt_completion_init(struct rt_completion *completion)
{
    PRT_SemDelete(completion->sem);
    PRT_SemCreate(0, &completion->sem);
}

/**
 * @brief This function will wait for a completion, if the completion is unavailable, the thread shall wait for
 *        the completion up to a specified time.
 *
 * @param completion is a pointer to a completion object.
 *
 * @param timeout is a timeout period (unit: OS ticks). If the completion is unavailable, the thread will wait for
 *                the completion done up to the amount of time specified by the argument.
 *                NOTE: Generally, we use the macro RT_WAITING_FOREVER to set this parameter, which means that when the
 *                completion is unavailable, the thread will be waitting forever.
 *
 * @return Return the operation status. ONLY when the return value is RT_EOK, the operation is successful.
 *         If the return value is any other values, it means that the completion wait failed.
 *
 * @warning This function can ONLY be called in the thread context. It MUST NOT be called in interrupt context.
 */
rt_err_t rt_completion_wait(struct rt_completion *completion,
                            rt_int32_t timeout)
{
    U32 tick = timeout;

    if (timeout == RT_WAITING_FOREVER)
    {
        tick = OS_WAIT_FOREVER;
    }

    if (PRT_SemPend(completion->sem, tick) == RT_EOK)
    {
        do
        {
        } while (PRT_SemPend(completion->sem, 0) == RT_EOK);

        return RT_EOK;
    }
    else
    {
        return -RT_ETIMEOUT;
    }
}

/**
 * @brief This function indicates a completion has done.
 *
 * @param completion is a pointer to a completion object.
 */
void rt_completion_done(struct rt_completion *completion)
{
    RT_ASSERT(completion != RT_NULL);

    PRT_SemPost(completion->sem);
}
