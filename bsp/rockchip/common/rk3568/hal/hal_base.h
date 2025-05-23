/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2021 Rockchip Electronics Co., Ltd.
 */

/** @addtogroup RK_HAL_Driver
 *  @{
 */

/** @addtogroup HAL_BASE
 *  @{
 */

#ifndef _HAL_BASE_H_
#define _HAL_BASE_H_

#include <prt_clk.h>
#include "hal_conf.h"
#include "hal_driver.h"
#include "hal_debug.h"

uint64_t HAL_DivU64Rem(uint64_t numerator, uint32_t denominator, uint32_t *pRemainder);

static inline uint64_t HAL_DivU64(uint64_t numerator, uint32_t denominator)
{
    return HAL_DivU64Rem(numerator, denominator, NULL);
}

static inline HAL_Status HAL_DelayMs(uint32_t ms)
{
    PRT_ClkDelayMs(ms);
    return HAL_OK;
}

static inline HAL_Status HAL_DelayUs(uint32_t us)
{
    PRT_ClkDelayUs(us);
    return HAL_OK;
}

static inline HAL_Status HAL_CPUDelayUs(uint32_t us)
{
    PRT_ClkDelayUs(us);
    return HAL_OK;
}

#endif

