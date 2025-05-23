/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: fxmac_phy.h
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:58
 * Description:  This file is for phy configuration.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/06/16    first release
 */

#ifndef FXMAC_PHY_H
#define FXMAC_PHY_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fxmac.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define XMAC_PHY_RESET_ENABLE  1
#define XMAC_PHY_RESET_DISABLE 0

enum
{
    FXMAC_PHY_AUTONEGOTIATION_DISABLE = 0,
    FXMAC_PHY_AUTONEGOTIATION_ENABLE
};

enum
{
    FXMAC_PHY_MODE_HALFDUPLEX = 0,
    FXMAC_PHY_MODE_FULLDUPLEX
};

/* phy interface */
FError FXmacPhyWrite(FXmac *instance_p, u32 phy_address, u32 register_num, u16 phy_data);
FError FXmacPhyRead(FXmac *instance_p, u32 phy_address, u32 register_num, u16 *phydat_aptr);
FError FXmacPhyInit(FXmac *instance_p, u32 speed, u32 duplex_mode, u32 autonegotiation_en, u32 reset_flag);

#ifdef __cplusplus
}
#endif

#endif // !