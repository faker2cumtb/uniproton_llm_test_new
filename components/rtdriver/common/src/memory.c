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
#include <prt_mem.h>

void *rt_malloc(size_t n)
{
    return PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, n);
}

void rt_free(void *p)
{
    PRT_MemFree(0, p);
}
