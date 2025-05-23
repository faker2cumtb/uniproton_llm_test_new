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
 * FilePath: fxmac_phy.c
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:58
 * Description:  This file is for phy types.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/06/16    first release
 */

#include "fxmac.h"
#include "eth_ieee_reg.h"
#include "fdrivers_port.h"
#include "fxmac_phy.h"
#include "prt_clk.h"
#if defined(CONFIG_FXMAC_PHY_YT)
    #include "phy_yt.h"
#endif


#define FXMAC_DEBUG_TAG "FXMAC_PHY"
#define FXMAC_ERROR(format, ...) FT_DEBUG_PRINT_E(FXMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_INFO(format, ...) FT_DEBUG_PRINT_I(FXMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_DEBUG(format, ...) FT_DEBUG_PRINT_D(FXMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_WARN(format, ...) FT_DEBUG_PRINT_W(FXMAC_DEBUG_TAG, format, ##__VA_ARGS__)

static FXmac *instance_b;
static u32 phy_addr_b;

/**
 * Write data to the specified PHY register. The Ethernet driver does not
 * require the device to be stopped before writing to the PHY.  Although it is
 * probably a good idea to stop the device, it is the responsibility of the
 * application to deem this necessary. The MAC provides the driver with the
 * ability to talk to a PHY that adheres to the Media Independent Interface
 * (MII) as defined in the IEEE 802.3 standard.
 *
 * Prior to PHY access with this function, the user should have setup the MDIO
 * clock with FXmacSetMdioDivisor().
 *
 * @param instance_p is a pointer to the FXmac instance to be worked on.
 * @param phy_address is the address of the PHY to be written (supports multiple
 *        PHYs)
 * @param register_num is the register number, 0-31, of the specific PHY register
 *        to write
 * @param phy_data is the 16-bit value that will be written to the register
 *
 * @return
 *
 * - FT_SUCCESS if the PHY was written to successfully. Since there is no error
 *   status from the MAC on a write, the user should read the PHY to verify the
 *   write was successful.
 * - FXMAC_ERR_PHY_BUSY if there is another PHY operation in progress
 *
 * @note
 *
 * This function is not thread-safe. The user must provide mutually exclusive
 * access to this function if there are to be multiple threads that can call it.
 *
 * There is the possibility that this function will not return if the hardware
 * is broken (i.e., it never sets the status bit indicating that the write is
 * done). If this is of concern to the user, the user should provide a mechanism
 * suitable to their needs for recovery.
 *
 * For the duration of this function, all host interface reads and writes are
 * blocked to the current FXmac instance.
 *
 ******************************************************************************/
FError FXmacPhyWrite(FXmac *instance_p, u32 phy_address, u32 register_num, u16 phy_data)
{
    u32 mgtcr;
    volatile u32 ipisr;
    u32 ip_write_temp;
    FError status;

    FASSERT(instance_p != NULL);

    /* Make sure no other PHY operation is currently in progress */
    if ((!(FXMAC_READREG32(instance_p->config.base_address,
                           FXMAC_NWSR_OFFSET) &
           FXMAC_NWSR_MDIOIDLE_MASK)) == TRUE)
    {
        status = (FError)(FXMAC_ERR_PHY_BUSY);
    }
    else
    {
        /* Construct mgtcr mask for the operation */
        mgtcr = FXMAC_PHYMNTNC_OP_MASK | FXMAC_PHYMNTNC_OP_W_MASK |
                (phy_address << FXMAC_PHYMNTNC_PHAD_SHFT_MSK) |
                (register_num << FXMAC_PHYMNTNC_PREG_SHFT_MSK) | (u32)phy_data;

        /* Write mgtcr and wait for completion */
        FXMAC_WRITEREG32(instance_p->config.base_address,
                         FXMAC_PHYMNTNC_OFFSET, mgtcr);

        do
        {
            ipisr = FXMAC_READREG32(instance_p->config.base_address,
                                    FXMAC_NWSR_OFFSET);
            ip_write_temp = ipisr;
        }
        while ((ip_write_temp & FXMAC_NWSR_MDIOIDLE_MASK) == 0x00000000U);

        status = (FError)(FT_SUCCESS);
    }
    return status;
}

/**
 * Read the current value of the PHY register indicated by the phy_address and
 * the register_num parameters. The MAC provides the driver with the ability to
 * talk to a PHY that adheres to the Media Independent Interface (MII) as
 * defined in the IEEE 802.3 standard.
 *
 *
 * @param instance_p is a pointer to the FXmac instance to be worked on.
 * @param phy_address is the address of the PHY to be read (supports multiple
 *        PHYs)
 * @param register_num is the register number, 0-31, of the specific PHY register
 *        to read
 * @param phydat_aptr is an output parameter, and points to a 16-bit buffer into
 *        which the current value of the register will be copied.
 *
 * @return
 *
 * - FT_SUCCESS if the PHY was read from successfully
 * - FXMAC_ERR_PHY_BUSY if there is another PHY operation in progress
 *
 * @note
 *
 * This function is not thread-safe. The user must provide mutually exclusive
 * access to this function if there are to be multiple threads that can call it.
 *
 * There is the possibility that this function will not return if the hardware
 * is broken (i.e., it never sets the status bit indicating that the read is
 * done). If this is of concern to the user, the user should provide a mechanism
 * suitable to their needs for recovery.
 *
 * For the duration of this function, all host interface reads and writes are
 * blocked to the current FXmac instance.
 *
 ******************************************************************************/
FError FXmacPhyRead(FXmac *instance_p, u32 phy_address,
                    u32 register_num, u16 *phydat_aptr)
{
    u32 mgtcr;
    volatile u32 ipisr;
    u32 IpReadTemp;
    FError status;

    FASSERT(instance_p != NULL);

    /* Make sure no other PHY operation is currently in progress */
    if ((!(FXMAC_READREG32(instance_p->config.base_address,
                           FXMAC_NWSR_OFFSET) &
           FXMAC_NWSR_MDIOIDLE_MASK)) == TRUE)
    {
        status = (FError)(FXMAC_ERR_PHY_BUSY);
    }
    else
    {
        /* Construct mgtcr mask for the operation */
        mgtcr = FXMAC_PHYMNTNC_OP_MASK | FXMAC_PHYMNTNC_OP_R_MASK |
                (phy_address << FXMAC_PHYMNTNC_PHAD_SHFT_MSK) |
                (register_num << FXMAC_PHYMNTNC_PREG_SHFT_MSK);

        /* Write mgtcr and wait for completion */
        FXMAC_WRITEREG32(instance_p->config.base_address,
                         FXMAC_PHYMNTNC_OFFSET, mgtcr);

        do
        {
            ipisr = FXMAC_READREG32(instance_p->config.base_address,
                                    FXMAC_NWSR_OFFSET);
            IpReadTemp = ipisr;
        }
        while ((IpReadTemp & FXMAC_NWSR_MDIOIDLE_MASK) == 0x00000000U);

        /* Read data */
        *phydat_aptr = (u16)FXMAC_READREG32(instance_p->config.base_address,
                                            FXMAC_PHYMNTNC_OFFSET);
        status = (FError)(FT_SUCCESS);
    }
    return status;
}

static FError FXmacDetect(FXmac *instance_p, u32 *phy_addr_p, u32 *phy_id_p)
{
    u32 phy_addr = 0;
    u16 phy_reg = 0, phy_id1_reg, phy_id2_reg;
    FError ret;
    instance_b = instance_p;

    for (phy_addr = 0; phy_addr < FXMAC_PHY_MAX_NUM; phy_addr++)
    {
        ret = FXmacPhyRead(instance_p, phy_addr, PHY_STATUS_REG_OFFSET, &phy_reg);
        if (ret != FT_SUCCESS)
        {
            FXMAC_ERROR("Phy operation is busy.");
            return ret;
        }
        FXMAC_INFO("Phy status reg is %x", phy_reg);
        if (phy_reg != 0xffff)
        {
            ret = FXmacPhyRead(instance_p, phy_addr, PHY_IDENTIFIER_1_REG, &phy_id1_reg);
            ret |= FXmacPhyRead(instance_p, phy_addr, PHY_IDENTIFIER_2_REG, &phy_id2_reg);
            *phy_id_p = phy_id1_reg<<16 | phy_id2_reg;
            FXMAC_INFO("Phy id1 reg is 0x%x, Phy id2 reg is 0x%x", phy_id1_reg , phy_id2_reg);
            if ((ret == FT_SUCCESS) && (phy_id2_reg != 0) && (phy_id2_reg != 0xffff) && (phy_id1_reg != 0xffff))
            {
                *phy_addr_p = phy_addr;
                phy_addr_b = phy_addr;
                FXMAC_INFO("Phy addr is 0x%x", phy_addr);
                return FT_SUCCESS;
            }
        }
    }

    return FXMAC_PHY_IS_NOT_FOUND;
}

/**
 * @name: FXmacPhyReset
 * @msg: Perform phy software reset
 * @param {FXmac} *instance_p, instance of FXmac controller
 * @param {u32} phy_addr, phy address connect to fxmac
 * @return err code information, FT_SUCCESS indicates success，others indicates failed
 */
static FError FXmacPhyReset(FXmac *instance_p, u32 phy_addr)
{
    u16 control;
    FError ret;

    ret = FXmacPhyRead(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, &control);
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,read PHY_CONTROL_REG_OFFSET is error", __func__, __LINE__);
        return ret;
    }

    control |= PHY_CONTROL_RESET_MASK;

    ret = FXmacPhyWrite(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, control);
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,write PHY_CONTROL_REG_OFFSET is error", __func__, __LINE__);
        return ret;
    }
     
    do
    {   
        ret = FXmacPhyRead(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, &control);
        if (ret != FT_SUCCESS)
        {
            FXMAC_ERROR("%s:%d,read PHY_CONTROL_REG_OFFSET is error", __func__, __LINE__);
            return ret;
        }

    }while((control & PHY_CONTROL_RESET_MASK) != 0);
    
    FXMAC_INFO(" Phy reset end.");
    return ret;
}

static FError FXmacGetIeeePhySpeed(FXmac *instance_p, u32 phy_addr, u32 phy_id)
{
    u16 temp;
    u16 control;
    u16 status;
    u32 negotitation_timeout_cnt = 0;
    FError ret;

    FXMAC_INFO("Start phy auto negotiation.");

    ret = FXmacPhyRead(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, &control);
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,read PHY_CONTROL_REG_OFFSET is error", __func__, __LINE__);
        return ret;
    }

    control |= PHY_CONTROL_AUTONEGOTIATE_ENABLE;
    control |= PHY_CONTROL_AUTONEGOTIATE_RESTART;
    ret = FXmacPhyWrite(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, control);
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,write PHY_CONTROL_REG_OFFSET is error", __func__, __LINE__);
        return ret;
    }

    FXMAC_INFO("Waiting for phy to complete auto negotiation.");

    do
    {
        PRT_ClkDelayMs(50);
        ret = FXmacPhyRead(instance_p, phy_addr, PHY_STATUS_REG_OFFSET, &status);
        if (ret != FT_SUCCESS)
        {
            FXMAC_ERROR("%s:%d,read PHY_STATUS_REG_OFFSET is error", __func__, __LINE__);
            return ret;
        }


        if (negotitation_timeout_cnt++ >= 0xff)
        {
            FXMAC_ERROR("Auto negotiation is error.");
            return FXMAC_PHY_AUTO_AUTONEGOTIATION_FAILED;
        }
    } while (!(status & PHY_STATUS_AUTONEGOTIATE_COMPLETE));
    
    FXMAC_INFO("Auto negotiation complete.");

    if(phy_id == PHY_ID_ADIN1200) {
        ret = FXmacPhyRead(instance_p, phy_addr, PHY_ADIN1200_SPECIFIC_STATUS_REG, &temp);
        if (ret != FT_SUCCESS)
        {
            FXMAC_ERROR("%s:%d,read PHY_SPECIFIC_STATUS_REG is error", __func__, __LINE__);
            return ret;
        }

        FXMAC_INFO("PHY_ID_ADIN1200 Temp is 0x%x", temp);
        if (temp & PHY_ADIN1200_SPECIFIC_STATUS_FULL_DUPLEX)
        {
            FXMAC_INFO("Duplex is full.");
            instance_p->config.duplex = 1;
        }
        else
        {
            FXMAC_INFO("Duplex is half.");
            instance_p->config.duplex = 0;
        }

        if (temp & PHY_ADIN1200_SPECIFIC_STATUS_SPEED_100M)
        {
            FXMAC_INFO("Speed is 100M.");
            instance_p->config.speed = 100;
        }
        else
        {
            FXMAC_INFO("Speed is 10M.dfis");
            instance_p->config.speed = 10;
        }
    }
    else
    {
        ret = FXmacPhyRead(instance_p, phy_addr, PHY_SPECIFIC_STATUS_REG, &temp);
        if (ret != FT_SUCCESS)
        {
            FXMAC_ERROR("%s:%d,read PHY_SPECIFIC_STATUS_REG is error", __func__, __LINE__);
            return ret;
        }
        FXMAC_INFO("Temp is 0x%x", temp);
        if (temp & (1 << 13))
        {
            FXMAC_INFO("Duplex is full.");
            instance_p->config.duplex = 1;
        }
        else
        {
            FXMAC_INFO("Duplex is half.");
            instance_p->config.duplex = 0;
        }

        if ((temp & 0xC000) == PHY_SPECIFIC_STATUS_SPEED_1000M)
        {
            FXMAC_INFO("Speed is 1000M.");
            instance_p->config.speed = 1000;
        }
        else if ((temp & 0xC000) == PHY_SPECIFIC_STATUS_SPEED_100M)
        {
            FXMAC_INFO("Speed is 100M.");
            instance_p->config.speed = 100;
        }
        else
        {
            FXMAC_INFO("Speed is 10Mzzz.");
            instance_p->config.speed = 10;
        }
    }

    return FT_SUCCESS;
}

static FError FXmacConfigureIeeePhySpeed(FXmac *instance_p, u32 phy_addr, u32 speed, u32 duplex_mode, u32 phy_id)
{
    u16 control;
    u16 autonereg;
    FError ret;
    u16 specific_reg = 0;

    FXMAC_INFO("Manual setting ,phy_addr is %d,speed %d, duplex_mode is %d.", phy_addr, speed, duplex_mode);

    ret = FXmacPhyRead(instance_p, phy_addr, PHY_AUTONEGO_ADVERTISE_REG, &autonereg);
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,read PHY_AUTONEGO_ADVERTISE_REG is error.", __func__, __LINE__);
        return ret;
    }

    autonereg |= PHY_AUTOADVERTISE_ASYMMETRIC_PAUSE_MASK;
    autonereg |= PHY_AUTOADVERTISE_PAUSE_MASK;
    ret = FXmacPhyWrite(instance_p, phy_addr, PHY_AUTONEGO_ADVERTISE_REG, autonereg);
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,write PHY_AUTONEGO_ADVERTISE_REG is error.", __func__, __LINE__);
        return ret;
    }


    ret = FXmacPhyRead(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, &control);
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,read PHY_AUTONEGO_ADVERTISE_REG is error.", __func__, __LINE__);
        return ret;
    }
    FXMAC_INFO("PHY_CONTROL_REG_OFFSET is 0x%x.", control);


    control &= ~PHY_CONTROL_LINKSPEED_1000M;
    control &= ~PHY_CONTROL_LINKSPEED_100M;
    control &= ~PHY_CONTROL_LINKSPEED_10M;

    if (speed == 100)
    {
        control |= PHY_CONTROL_LINKSPEED_100M;
    }
    else if (speed == 10)
    {
        control |= PHY_CONTROL_LINKSPEED_10M;
    }

    if (duplex_mode == 1)
    {
        control |= PHY_CONTROL_FULL_DUPLEX_MASK;
    }
    else
    {
        control &= ~PHY_CONTROL_FULL_DUPLEX_MASK;
    }

    /* disable auto-negotiation */
    control &= ~(PHY_CONTROL_AUTONEGOTIATE_ENABLE);
    control &= ~(PHY_CONTROL_AUTONEGOTIATE_RESTART);

    ret = FXmacPhyWrite(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, control); /* Technology Ability Field */
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,write PHY_AUTONEGO_ADVERTISE_REG is error.", __func__, __LINE__);
        return ret;
    }

    PRT_ClkDelayMs(1500);

    FXMAC_INFO("Manual selection completed.");

    if(phy_id == PHY_ID_ADIN1200)
    {
		return FT_SUCCESS;
	}

    ret = FXmacPhyRead(instance_p, phy_addr, PHY_SPECIFIC_STATUS_REG, &specific_reg);
    if (ret != FT_SUCCESS)
    {
        FXMAC_ERROR("%s:%d,read PHY_SPECIFIC_STATUS_REG is error.", __func__, __LINE__);
        return ret;
    }

    FXMAC_INFO("Specific reg is 0x%x.", specific_reg);

    if (specific_reg & (1 << 13))
    {
        FXMAC_INFO("Duplex is full.");
        instance_p->config.duplex = 1;
    }
    else
    {
        FXMAC_INFO("Duplex is half.");
        instance_p->config.duplex = 0;
    }

    if ((specific_reg & 0xC000) == PHY_SPECIFIC_STATUS_SPEED_100M)
    {
        FXMAC_INFO("Speed is 100M.");
        instance_p->config.speed = 100;
    }
    else
    {
        FXMAC_INFO("Speed is 10M.");
        instance_p->config.speed = 10;
    }

    
    return FT_SUCCESS;
}

/**
 * @name: FXmacPhyInit
 * @msg:  setup the PHYs for proper speed setting.
 * @param {FXmac} *instance_p is a pointer to the instance to be worked on.
 * @param {u32} speed is phy operating speed
 * @param {u32} phy_addr is the address of the PHY to be read (supports multiple PHYs)
 * @param {u32} duplex_mode is The duplex mode can be selected via either the Auto-Negotiation process or manual duplex selection.
 * @param {u32} autonegotiation_en is an auto-negotiated flag . 1 is enable auto ,0 is manual
 * @param {u32} reset_flag is a flag which indicates whether to reset xmac phy.
 * @return {FError}
 */
FError FXmacPhyInit(FXmac *instance_p, u32 speed, u32 duplex_mode, u32 autonegotiation_en, u32 reset_flag)
{
    FError ret;
    u32 phy_addr;
    u32 phy_id;

    if (FXmacDetect(instance_p, &phy_addr, &phy_id) != FT_SUCCESS)
    {
        FXMAC_ERROR("Phy is not found.");
        return FXMAC_PHY_IS_NOT_FOUND;
    }

    FXMAC_INFO("Setting phy addr is %d.", phy_addr);
    instance_p->phy_address = phy_addr;
    if(reset_flag)
    {
        FXmacPhyReset(instance_p,phy_addr);
    }
   
    if (autonegotiation_en)
    {
        ret = FXmacGetIeeePhySpeed(instance_p, phy_addr, phy_id);
        if (ret != FT_SUCCESS)
        {
            return ret;
        }
    }
    else
    {
        FXMAC_INFO("Set the communication speed manually.");
        FASSERT_MSG(speed != FXMAC_SPEED_1000,"The speed must be 100M or 10M!");
        ret = FXmacConfigureIeeePhySpeed(instance_p, phy_addr, speed, duplex_mode, phy_id);
        if (ret != FT_SUCCESS)
        {
            FXMAC_ERROR("Failed to manually set the phy.");
            return ret;
        }    

    }

    instance_p->link_status = FXMAC_LINKUP;
    return FT_SUCCESS;
}
