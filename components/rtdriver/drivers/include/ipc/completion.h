/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-18     LuoYuncong   Port to UniProton
 */
#ifndef COMPLETION_H_
#define COMPLETION_H_

#include <rtapi.h>
#include <prt_sem.h>

struct rt_completion
{
    SemHandle sem;
    rt_uint8_t res[2];
};

void rt_completion_init(struct rt_completion *completion);
rt_err_t rt_completion_wait(struct rt_completion *completion,
                            rt_int32_t            timeout);
void rt_completion_done(struct rt_completion *completion);

#endif
