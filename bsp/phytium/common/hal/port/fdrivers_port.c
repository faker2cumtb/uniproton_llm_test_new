/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
 * All Rights Reserved.
 * 
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 * 
 * 
 * FilePath: fdrivers_port.c
 * Created Date: 2023-10-17 08:29:18
 * Last Modified: 2023-11-21 17:03:55
 * Description:  This file is for drive layer code decoupling
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/10/17    first release
 */
#include "fdrivers_port.h"
#include "prt_clk.h"

/* time delay */

void FDriverUdelay(u32 usec)
{
    PRT_ClkCycle2Us(usec);
}

void FDriverMdelay(u32 msec)
{
    PRT_ClkCycle2Ms(msec);
}
