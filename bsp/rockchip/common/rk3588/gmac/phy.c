/*
 * Generic PHY Management code
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright 2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 *
 * Based loosely off of Linux's PHY Lib
 */

#include <stdint.h>
#include <prt_typedef.h>
#include "hal_base.h"
#include "bitops.h"
#include "phy.h"
#include "miiphy.h"
#include "errno2.h"

/* Generic PHY support and helper functions */

/**
 * genphy_config_advert - sanitize and advertise auto-negotation parameters
 * @phydev: target phy_device struct
 *
 * Description: Writes MII_ADVERTISE with the appropriate values,
 *   after sanitizing the values to make sure we only advertise
 *   what is supported.  Returns < 0 on error, 0 if the PHY's advertisement
 *   hasn't changed, and > 0 if it has changed.
 */
static int genphy_config_advert(struct phy_device *phydev)
{
    uint32_t advertise;
    int oldadv, adv, bmsr;
    int err, changed = 0;

    /* Only allow advertising what this PHY supports */
    phydev->advertising &= phydev->supported;
    advertise = phydev->advertising;

    /* Setup standard advertisement */
    adv = phy_read(phydev, MDIO_DEVAD_NONE, MII_ADVERTISE);
    oldadv = adv;

    if (adv < 0)
        return adv;

    adv &= ~(ADVERTISE_ALL | ADVERTISE_100BASE4 | ADVERTISE_PAUSE_CAP |
             ADVERTISE_PAUSE_ASYM);
    if (advertise & ADVERTISED_10baseT_Half)
        adv |= ADVERTISE_10HALF;
    if (advertise & ADVERTISED_10baseT_Full)
        adv |= ADVERTISE_10FULL;
    if (advertise & ADVERTISED_100baseT_Half)
        adv |= ADVERTISE_100HALF;
    if (advertise & ADVERTISED_100baseT_Full)
        adv |= ADVERTISE_100FULL;
    if (advertise & ADVERTISED_Pause)
        adv |= ADVERTISE_PAUSE_CAP;
    if (advertise & ADVERTISED_Asym_Pause)
        adv |= ADVERTISE_PAUSE_ASYM;
    if (advertise & ADVERTISED_1000baseX_Half)
        adv |= ADVERTISE_1000XHALF;
    if (advertise & ADVERTISED_1000baseX_Full)
        adv |= ADVERTISE_1000XFULL;

    if (adv != oldadv)
    {
        err = phy_write(phydev, MDIO_DEVAD_NONE, MII_ADVERTISE, adv);

        if (err < 0)
            return err;
        changed = 1;
    }

    bmsr = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);
    if (bmsr < 0)
        return bmsr;

    /* Per 802.3-2008, Section 22.2.4.2.16 Extended status all
     * 1000Mbits/sec capable PHYs shall have the BMSR_ESTATEN bit set to a
     * logical 1.
     */
    if (!(bmsr & BMSR_ESTATEN))
        return changed;

    /* Configure gigabit if it's supported */
    adv = phy_read(phydev, MDIO_DEVAD_NONE, MII_CTRL1000);
    oldadv = adv;

    if (adv < 0)
        return adv;

    adv &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);

    if (phydev->supported & (SUPPORTED_1000baseT_Half |
                             SUPPORTED_1000baseT_Full))
    {
        if (advertise & SUPPORTED_1000baseT_Half)
            adv |= ADVERTISE_1000HALF;
        if (advertise & SUPPORTED_1000baseT_Full)
            adv |= ADVERTISE_1000FULL;
    }

    if (adv != oldadv)
        changed = 1;

    err = phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, adv);
    if (err < 0)
        return err;

    return changed;
}

/**
 * genphy_setup_forced - configures/forces speed/duplex from @phydev
 * @phydev: target phy_device struct
 *
 * Description: Configures MII_BMCR to force speed/duplex
 *   to the values in phydev. Assumes that the values are valid.
 */
static int genphy_setup_forced(struct phy_device *phydev)
{
    int err;
    int ctl = BMCR_ANRESTART;

    phydev->pause = phydev->asym_pause = 0;

    if (SPEED_1000 == phydev->speed)
        ctl |= BMCR_SPEED1000;
    else if (SPEED_100 == phydev->speed)
        ctl |= BMCR_SPEED100;

    if (DUPLEX_FULL == phydev->duplex)
        ctl |= BMCR_FULLDPLX;

    err = phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, ctl);

    return err;
}

/**
 * genphy_restart_aneg - Enable and Restart Autonegotiation
 * @phydev: target phy_device struct
 */
int genphy_restart_aneg(struct phy_device *phydev)
{
    int ctl;

    ctl = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

    if (ctl < 0)
        return ctl;

    ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);

    /* Don't isolate the PHY if we're negotiating */
    ctl &= ~(BMCR_ISOLATE);

    ctl = phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, ctl);

    return ctl;
}

/**
 * genphy_config_aneg - restart auto-negotiation or write BMCR
 * @phydev: target phy_device struct
 *
 * Description: If auto-negotiation is enabled, we configure the
 *   advertising, and then restart auto-negotiation.  If it is not
 *   enabled, then we write the BMCR.
 */
int genphy_config_aneg(struct phy_device *phydev)
{
    int result;

    if (AUTONEG_ENABLE != phydev->autoneg)
        return genphy_setup_forced(phydev);

    result = genphy_config_advert(phydev);

    if (result < 0) /* error */
        return result;

    if (result == 0)
    {
        /* Advertisment hasn't changed, but maybe aneg was never on to
         * begin with?  Or maybe phy was isolated? */
        int ctl = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

        if (ctl < 0)
            return ctl;

        if (!(ctl & BMCR_ANENABLE) || (ctl & BMCR_ISOLATE))
            result = 1; /* do restart aneg */
    }

    /* Only restart aneg if we are advertising something different
     * than we were before.	 */
    if (result > 0)
        result = genphy_restart_aneg(phydev);

    return result;
}

/**
 * genphy_update_link - update link status in @phydev
 * @phydev: target phy_device struct
 *
 * Description: Update the value in phydev->link to reflect the
 *   current link value.  In order to do this, we need to read
 *   the status register twice, keeping the second value.
 */
int genphy_update_link(struct phy_device *phydev)
{
    unsigned int mii_reg;

    /*
     * Wait if the link is up, and autonegotiation is in progress
     * (ie - we're capable and it's not done)
     */
    mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

    /*
     * If we already saw the link up, and it hasn't gone down, then
     * we don't need to wait for autoneg again
     */
    if (phydev->link && mii_reg & BMSR_LSTATUS)
        return 0;

    if ((phydev->autoneg == AUTONEG_ENABLE) &&
        !(mii_reg & BMSR_ANEGCOMPLETE))
    {
        int i = 0;

        HAL_DBG("Waiting for PHY auto negotiation to complete");
        while (!(mii_reg & BMSR_ANEGCOMPLETE))
        {
            /*
             * Timeout reached ?
             */
            if (i > PHY_ANEG_TIMEOUT)
            {
                HAL_DBG(" TIMEOUT !\n");
                phydev->link = 0;
                return -ETIMEDOUT;
            }

            if ((i++ % 500) == 0)
                HAL_SYSLOG(".");

            HAL_DelayUs(1000); /* 1 ms */
            mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);
        }
        HAL_SYSLOG(" done\n");
        phydev->link = 1;
    }
    else
    {
        /* Read the link a second time to clear the latched state */
        mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

        if (mii_reg & BMSR_LSTATUS)
            phydev->link = 1;
        else
            phydev->link = 0;
    }

    return 0;
}

/*
 * Generic function which updates the speed and duplex.  If
 * autonegotiation is enabled, it uses the AND of the link
 * partner's advertised capabilities and our advertised
 * capabilities.  If autonegotiation is disabled, we use the
 * appropriate bits in the control register.
 *
 * Stolen from Linux's mii.c and phy_device.c
 */
int genphy_parse_link(struct phy_device *phydev)
{
    int mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

    /* We're using autonegotiation */
    if (phydev->autoneg == AUTONEG_ENABLE)
    {
        uint32_t lpa = 0;
        int gblpa = 0;
        uint32_t estatus = 0;

        /* Check for gigabit capability */
        if (phydev->supported & (SUPPORTED_1000baseT_Full |
                                 SUPPORTED_1000baseT_Half))
        {
            /* We want a list of states supported by
             * both PHYs in the link
             */
            gblpa = phy_read(phydev, MDIO_DEVAD_NONE, MII_STAT1000);
            if (gblpa < 0)
            {
                HAL_DBG("Could not read MII_STAT1000. Ignoring gigabit capability\n");
                gblpa = 0;
            }
            gblpa &= phy_read(phydev,
                              MDIO_DEVAD_NONE, MII_CTRL1000)
                     << 2;
        }

        /* Set the baseline so we only have to set them
         * if they're different
         */
        phydev->speed = SPEED_10;
        phydev->duplex = DUPLEX_HALF;

        /* Check the gigabit fields */
        if (gblpa & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD))
        {
            phydev->speed = SPEED_1000;

            if (gblpa & PHY_1000BTSR_1000FD)
                phydev->duplex = DUPLEX_FULL;

            /* We're done! */
            return 0;
        }

        lpa = phy_read(phydev, MDIO_DEVAD_NONE, MII_ADVERTISE);
        lpa &= phy_read(phydev, MDIO_DEVAD_NONE, MII_LPA);

        if (lpa & (LPA_100FULL | LPA_100HALF))
        {
            phydev->speed = SPEED_100;

            if (lpa & LPA_100FULL)
                phydev->duplex = DUPLEX_FULL;
        }
        else if (lpa & LPA_10FULL)
            phydev->duplex = DUPLEX_FULL;

        /*
         * Extended status may indicate that the PHY supports
         * 1000BASE-T/X even though the 1000BASE-T registers
         * are missing. In this case we can't tell whether the
         * peer also supports it, so we only check extended
         * status if the 1000BASE-T registers are actually
         * missing.
         */
        if ((mii_reg & BMSR_ESTATEN) && !(mii_reg & BMSR_ERCAP))
            estatus = phy_read(phydev, MDIO_DEVAD_NONE,
                               MII_ESTATUS);

        if (estatus & (ESTATUS_1000_XFULL | ESTATUS_1000_XHALF |
                       ESTATUS_1000_TFULL | ESTATUS_1000_THALF))
        {
            phydev->speed = SPEED_1000;
            if (estatus & (ESTATUS_1000_XFULL | ESTATUS_1000_TFULL))
                phydev->duplex = DUPLEX_FULL;
        }
    }
    else
    {
        uint32_t bmcr = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

        phydev->speed = SPEED_10;
        phydev->duplex = DUPLEX_HALF;

        if (bmcr & BMCR_FULLDPLX)
            phydev->duplex = DUPLEX_FULL;

        if (bmcr & BMCR_SPEED1000)
            phydev->speed = SPEED_1000;
        else if (bmcr & BMCR_SPEED100)
            phydev->speed = SPEED_100;
    }

    return 0;
}

int genphy_config(struct phy_device *phydev)
{
    int val;
    uint32_t features;

    features = (SUPPORTED_TP | SUPPORTED_MII | SUPPORTED_AUI | SUPPORTED_FIBRE |
                SUPPORTED_BNC);

    /* Do we support autonegotiation? */
    val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

    if (val < 0)
        return val;

    if (val & BMSR_ANEGCAPABLE)
        features |= SUPPORTED_Autoneg;

    if (val & BMSR_100FULL)
        features |= SUPPORTED_100baseT_Full;
    if (val & BMSR_100HALF)
        features |= SUPPORTED_100baseT_Half;
    if (val & BMSR_10FULL)
        features |= SUPPORTED_10baseT_Full;
    if (val & BMSR_10HALF)
        features |= SUPPORTED_10baseT_Half;

    if (val & BMSR_ESTATEN)
    {
        val = phy_read(phydev, MDIO_DEVAD_NONE, MII_ESTATUS);

        if (val < 0)
            return val;

        if (val & ESTATUS_1000_TFULL)
            features |= SUPPORTED_1000baseT_Full;
        if (val & ESTATUS_1000_THALF)
            features |= SUPPORTED_1000baseT_Half;
        if (val & ESTATUS_1000_XFULL)
            features |= SUPPORTED_1000baseX_Full;
        if (val & ESTATUS_1000_XHALF)
            features |= SUPPORTED_1000baseX_Half;
    }

    phydev->supported &= features;
    phydev->advertising &= features;

    genphy_config_aneg(phydev);

    return 0;
}

int genphy_startup(struct phy_device *phydev)
{
    int ret;

    ret = genphy_update_link(phydev);
    if (ret)
        return ret;

    return genphy_parse_link(phydev);
}

int genphy_shutdown(struct phy_device *phydev)
{
    return 0;
}

static struct phy_driver genphy_driver = {
    .uid = 0xffffffff,
    .mask = 0xffffffff,
    .name = "Generic PHY",
    .features = PHY_GBIT_FEATURES | SUPPORTED_MII |
                SUPPORTED_AUI | SUPPORTED_FIBRE |
                SUPPORTED_BNC,
    .config = genphy_config,
    .startup = genphy_startup,
    .shutdown = genphy_shutdown,
};

int phy_set_supported(struct phy_device *phydev, uint32_t max_speed)
{
    /* The default values for phydev->supported are provided by the PHY
     * driver "features" member, we want to reset to sane defaults first
     * before supporting higher speeds.
     */
    phydev->supported &= PHY_DEFAULT_FEATURES;

    switch (max_speed)
    {
    default:
        return -ENOTSUPP;
    case SPEED_1000:
        phydev->supported |= PHY_1000BT_FEATURES;
        /* fall through */
    case SPEED_100:
        phydev->supported |= PHY_100BT_FEATURES;
        /* fall through */
    case SPEED_10:
        phydev->supported |= PHY_10BT_FEATURES;
    }

    return 0;
}

static int phy_probe(struct phy_device *phydev)
{
    int err = 0;

    phydev->advertising = phydev->supported = phydev->drv->features;
    phydev->mmds = phydev->drv->mmds;

    if (phydev->drv->probe)
        err = phydev->drv->probe(phydev);

    return err;
}

static struct phy_driver *generic_for_interface(phy_interface_t interface)
{
    return &genphy_driver;
}

static struct phy_driver *get_phy_driver(struct phy_device *phydev,
                                         phy_interface_t interface)
{
    return generic_for_interface(interface);
}

static struct phy_device *phy_device_create(struct mii_dev *bus, int addr,
                                            uint32_t phy_id, bool is_c45,
                                            phy_interface_t interface)
{
    struct phy_device *dev;

    /* We allocate the device, and initialize the
     * default values */
    dev = malloc(sizeof(*dev));
    if (!dev)
    {
        HAL_DBG("Failed to allocate PHY device for %s:%d\n",
                bus->name, addr);
        return NULL;
    }

    memset(dev, 0, sizeof(*dev));

    dev->duplex = -1;
    dev->link = 0;
    dev->interface = interface;

#ifdef CONFIG_DM_ETH
    dev->node = ofnode_null();
#endif

    dev->autoneg = AUTONEG_ENABLE;

    dev->addr = addr;
    dev->phy_id = phy_id;
    dev->is_c45 = is_c45;
    dev->bus = bus;

    dev->drv = get_phy_driver(dev, interface);

    phy_probe(dev);

    bus->phymap[addr] = dev;

    return dev;
}

/**
 * get_phy_id - reads the specified addr for its ID.
 * @bus: the target MII bus
 * @addr: PHY address on the MII bus
 * @phy_id: where to store the ID retrieved.
 *
 * Description: Reads the ID registers of the PHY at @addr on the
 *   @bus, stores it in @phy_id and returns zero on success.
 */
int get_phy_id(struct mii_dev *bus, int addr, int devad, uint32_t *phy_id)
{
    int phy_reg;

    /* Grab the bits from PHYIR1, and put them
     * in the upper half */
    phy_reg = bus->read(bus, addr, devad, MII_PHYSID1);

    if (phy_reg < 0)
        return -EIO;

    *phy_id = (phy_reg & 0xffff) << 16;

    /* Grab the bits from PHYIR2, and put them in the lower half */
    phy_reg = bus->read(bus, addr, devad, MII_PHYSID2);

    if (phy_reg < 0)
        return -EIO;

    *phy_id |= (phy_reg & 0xffff);

    return 0;
}

static struct phy_device *create_phy_by_mask(struct mii_dev *bus,
                                             unsigned phy_mask, int devad, phy_interface_t interface)
{
    uint32_t phy_id = 0xffffffff;
    bool is_c45;

    while (phy_mask)
    {
        int addr = generic_ffs(phy_mask) - 1;
        int r = get_phy_id(bus, addr, devad, &phy_id);
        /* If the PHY ID is mostly f's, we didn't find anything */
        if (r == 0 && (phy_id & 0x1fffffff) != 0x1fffffff)
        {
            is_c45 = (devad == MDIO_DEVAD_NONE) ? false : true;
            return phy_device_create(bus, addr, phy_id, is_c45, interface);
        }
        phy_mask &= ~(1 << addr);
    }
    return NULL;
}

static struct phy_device *search_for_existing_phy(struct mii_dev *bus,
                                                  unsigned phy_mask, phy_interface_t interface)
{
    /* If we have one, return the existing device, with new interface */
    while (phy_mask)
    {
        int addr = generic_ffs(phy_mask) - 1;
        if (bus->phymap[addr])
        {
            bus->phymap[addr]->interface = interface;
            return bus->phymap[addr];
        }
        phy_mask &= ~(1 << addr);
    }
    return NULL;
}

static struct phy_device *get_phy_device_by_mask(struct mii_dev *bus,
                                                 unsigned phy_mask, phy_interface_t interface)
{
    int i;
    struct phy_device *phydev;

    phydev = search_for_existing_phy(bus, phy_mask, interface);
    if (phydev)
        return phydev;
    /* Try Standard (ie Clause 22) access */
    /* Otherwise we have to try Clause 45 */
    for (i = 0; i < 5; i++)
    {
        phydev = create_phy_by_mask(bus, phy_mask, i ? i : MDIO_DEVAD_NONE, interface);
        if (phydev)
            return phydev;
    }

    HAL_DBG("\n%s PHY: ", bus->name);
    while (phy_mask)
    {
        int addr = generic_ffs(phy_mask) - 1;
        HAL_DBG("%d ", addr);
        phy_mask &= ~(1 << addr);
    }
    HAL_DBG("not found\n");

    return NULL;
}

// /**
//  * get_phy_device - reads the specified PHY device and returns its @phy_device struct
//  * @bus: the target MII bus
//  * @addr: PHY address on the MII bus
//  *
//  * Description: Reads the ID registers of the PHY at @addr on the
//  *   @bus, then allocates and returns the phy_device to represent it.
//  */
// static struct phy_device *get_phy_device(struct mii_dev *bus, int addr,
//                                          phy_interface_t interface)
// {
//     return get_phy_device_by_mask(bus, 1 << addr, interface);
// }

int phy_reset(struct phy_device *phydev)
{
    int reg;
    int timeout = 500;
    int devad = MDIO_DEVAD_NONE;

    if (phydev->flags & PHY_FLAG_BROKEN_RESET)
        return 0;

#ifdef CONFIG_PHYLIB_10G
    /* If it's 10G, we need to issue reset through one of the MMDs */
    if (is_10g_interface(phydev->interface))
    {
        if (!phydev->mmds)
            gen10g_discover_mmds(phydev);

        devad = generic_ffs(phydev->mmds) - 1;
    }
#endif

    if (phy_write(phydev, devad, MII_BMCR, BMCR_RESET) < 0)
    {
        HAL_DBG("PHY reset failed\n");
        return -1;
    }

    /*
     * Poll the control register for the reset bit to go to 0 (it is
     * auto-clearing).  This should happen within 0.5 seconds per the
     * IEEE spec.
     */
    reg = phy_read(phydev, devad, MII_BMCR);
    while ((reg & BMCR_RESET) && timeout--)
    {
        reg = phy_read(phydev, devad, MII_BMCR);

        if (reg < 0)
        {
            HAL_DBG("PHY status read failed\n");
            return -1;
        }
        HAL_DelayUs(1000);
    }

    if (reg & BMCR_RESET)
    {
        puts("PHY reset timed out\n");
        return -1;
    }

    return 0;
}

struct phy_device *phy_find_by_mask(struct mii_dev *bus, unsigned phy_mask,
                                    phy_interface_t interface)
{
    /* Reset the bus */
    if (bus->reset)
    {
        bus->reset(bus);

        /* Wait 15ms to make sure the PHY has come out of hard reset */
        HAL_DelayUs(15000);
    }

    return get_phy_device_by_mask(bus, phy_mask, interface);
}

#ifdef CONFIG_DM_ETH
void phy_connect_dev(struct phy_device *phydev, struct udevice *dev)
#else
void phy_connect_dev(struct phy_device *phydev)
#endif
{
    /* Soft Reset the PHY */
    phy_reset(phydev);
}

#ifdef CONFIG_DM_ETH
struct phy_device *phy_connect(struct mii_dev *bus, int addr,
                               struct udevice *dev, phy_interface_t interface)
#else
struct phy_device *phy_connect(struct mii_dev *bus, int addr, phy_interface_t interface)
#endif
{
    struct phy_device *phydev = NULL;
#ifdef CONFIG_PHY_FIXED
    int sn;
    const char *name;
    sn = fdt_first_subnode(gd->fdt_blob, dev_of_offset(dev));
    while (sn > 0)
    {
        name = fdt_get_name(gd->fdt_blob, sn, NULL);
        if (name && strcmp(name, "fixed-link") == 0)
        {
            phydev = phy_device_create(bus, sn, PHY_FIXED_ID, false,
                                       interface);
            break;
        }
        sn = fdt_next_subnode(gd->fdt_blob, sn);
    }
#endif
    if (phydev == NULL)
        phydev = phy_find_by_mask(bus, 1 << addr, interface);

    if (phydev)
        phy_connect_dev(phydev);
    else
        HAL_DBG("Could not get PHY for %s: addr %d\n", bus->name, addr);
    return phydev;
}

/*
 * Start the PHY.  Returns 0 on success, or a negative error code.
 */
int phy_startup(struct phy_device *phydev)
{
    if (phydev->drv->startup)
    	return phydev->drv->startup(phydev);

    return 0;
}

int board_phy_config(struct phy_device *phydev)
{
    if (phydev->drv->config)
    	return phydev->drv->config(phydev);
    return 0;
}

int phy_config(struct phy_device *phydev)
{
    /* Invoke an optional board-specific helper */
    return board_phy_config(phydev);
}

int phy_shutdown(struct phy_device *phydev)
{
    if (phydev->drv->shutdown)
    	phydev->drv->shutdown(phydev);

    return 0;
}

