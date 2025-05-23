/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *	Andy Fleming <afleming@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * This file pretty much stolen from Linux's mii.h/ethtool.h/phy.h
 */

#ifndef _PHY_H
#define _PHY_H

#include <stdint.h>
#include "mii.h"
#include "ethtool.h"
#include "mdio.h"
#include "phy_interface.h"

#ifndef uchar
typedef unsigned char   uchar;
#endif

#ifndef ulong
typedef unsigned long   ulong;
#endif

#ifndef bool
typedef int             bool;
#endif

#define PHY_FIXED_ID		0xa5a55a5a

#define PHY_MAX_ADDR 32

#define PHY_FLAG_BROKEN_RESET	(1 << 0) /* soft reset not supported */

#define PHY_DEFAULT_FEATURES	(SUPPORTED_Autoneg | \
				 SUPPORTED_TP | \
				 SUPPORTED_MII)

#define PHY_10BT_FEATURES	(SUPPORTED_10baseT_Half | \
				 SUPPORTED_10baseT_Full)

#define PHY_100BT_FEATURES	(SUPPORTED_100baseT_Half | \
				 SUPPORTED_100baseT_Full)

#define PHY_1000BT_FEATURES	(SUPPORTED_1000baseT_Half | \
				 SUPPORTED_1000baseT_Full)

#define PHY_BASIC_FEATURES	(PHY_10BT_FEATURES | \
				 PHY_100BT_FEATURES | \
				 PHY_DEFAULT_FEATURES)

#define PHY_GBIT_FEATURES	(PHY_BASIC_FEATURES | \
				 PHY_1000BT_FEATURES)

#define PHY_10G_FEATURES	(PHY_GBIT_FEATURES | \
				SUPPORTED_10000baseT_Full)

#ifndef PHY_ANEG_TIMEOUT
#define PHY_ANEG_TIMEOUT	4000
#endif


struct phy_device;

#define MDIO_NAME_LEN 32

struct mii_dev {
	char name[MDIO_NAME_LEN];
	void *priv;
	int (*read)(struct mii_dev *bus, int addr, int devad, int reg);
	int (*write)(struct mii_dev *bus, int addr, int devad, int reg,
			uint16_t val);
	int (*reset)(struct mii_dev *bus);
	struct phy_device *phymap[PHY_MAX_ADDR];
	uint32_t phy_mask;
};

/* struct phy_driver: a structure which defines PHY behavior
 *
 * uid will contain a number which represents the PHY.  During
 * startup, the driver will poll the PHY to find out what its
 * UID--as defined by registers 2 and 3--is.  The 32-bit result
 * gotten from the PHY will be masked to
 * discard any bits which may change based on revision numbers
 * unimportant to functionality
 *
 */
struct phy_driver {
	char *name;
	unsigned int uid;
	unsigned int mask;
	unsigned int mmds;

	uint32_t features;

	/* Called to do any driver startup necessities */
	/* Will be called during phy_connect */
	int (*probe)(struct phy_device *phydev);

	/* Called to configure the PHY, and modify the controller
	 * based on the results.  Should be called after phy_connect */
	int (*config)(struct phy_device *phydev);

	/* Called when starting up the controller */
	int (*startup)(struct phy_device *phydev);

	/* Called when bringing down the controller */
	int (*shutdown)(struct phy_device *phydev);

	int (*readext)(struct phy_device *phydev, int addr, int devad, int reg);
	int (*writeext)(struct phy_device *phydev, int addr, int devad, int reg,
			uint16_t val);

	/* Phy specific driver override for reading a MMD register */
	int (*read_mmd)(struct phy_device *phydev, int devad, int reg);

	/* Phy specific driver override for writing a MMD register */
	int (*write_mmd)(struct phy_device *phydev, int devad, int reg,
			 uint16_t val);
};

struct phy_device {
	/* Information about the PHY type */
	/* And management functions */
	struct mii_dev *bus;
	struct phy_driver *drv;
	void *priv;

#ifdef CONFIG_DM_ETH
	struct udevice *dev;
	ofnode node;
#else
	struct eth_device *dev;
#endif

	/* forced speed & duplex (no autoneg)
	 * partner speed & duplex & pause (autoneg)
	 */
	int speed;
	int duplex;

	/* The most recently read link state */
	int link;
	int port;
	phy_interface_t interface;

	uint32_t advertising;
	uint32_t supported;
	uint32_t mmds;

	int autoneg;
	int addr;
	int pause;
	int asym_pause;
	uint32_t phy_id;
	bool is_c45;
	uint32_t flags;
};

struct fixed_link {
	int phy_id;
	int duplex;
	int link_speed;
	int pause;
	int asym_pause;
};

static inline int phy_read(struct phy_device *phydev, int devad, int regnum)
{
	struct mii_dev *bus = phydev->bus;

	return bus->read(bus, phydev->addr, devad, regnum);
}

static inline int phy_write(struct phy_device *phydev, int devad, int regnum,
			uint16_t val)
{
	struct mii_dev *bus = phydev->bus;

	return bus->write(bus, phydev->addr, devad, regnum, val);
}

/**
 * phy_init() - Initializes the PHY drivers
 *
 * This function registers all available PHY drivers
 *
 * @return 0 if OK, -ve on error
 */
int phy_init(void);

/**
 * phy_reset() - Resets the specified PHY
 *
 * Issues a reset of the PHY and waits for it to complete
 *
 * @phydev:	PHY to reset
 * @return 0 if OK, -ve on error
 */
int phy_reset(struct phy_device *phydev);

/**
 * phy_find_by_mask() - Searches for a PHY on the specified MDIO bus
 *
 * The function checks the PHY addresses flagged in phy_mask and returns a
 * phy_device pointer if it detects a PHY.
 * This function should only be called if just one PHY is expected to be present
 * in the set of addresses flagged in phy_mask.  If multiple PHYs are present,
 * it is undefined which of these PHYs is returned.
 *
 * @bus:	MII/MDIO bus to scan
 * @phy_mask:	bitmap of PYH addresses to scan
 * @interface:	type of MAC-PHY interface
 * @return pointer to phy_device if a PHY is found, or NULL otherwise
 */
struct phy_device *phy_find_by_mask(struct mii_dev *bus, unsigned phy_mask,
		phy_interface_t interface);

#ifdef CONFIG_DM_ETH

/**
 * phy_connect_dev() - Associates the given pair of PHY and Ethernet devices
 * @phydev:	PHY device
 * @dev:	Ethernet device
 */
void phy_connect_dev(struct phy_device *phydev, struct udevice *dev);

/**
 * phy_connect() - Creates a PHY device for the Ethernet interface
 *
 * Creates a PHY device for the PHY at the given address, if one doesn't exist
 * already, and associates it with the Ethernet device.
 * The function may be called with addr <= 0, in this case addr value is ignored
 * and the bus is scanned to detect a PHY.  Scanning should only be used if only
 * one PHY is expected to be present on the MDIO bus, otherwise it is undefined
 * which PHY is returned.
 *
 * @bus:	MII/MDIO bus that hosts the PHY
 * @addr:	PHY address on MDIO bus
 * @dev:	Ethernet device to associate to the PHY
 * @interface:	type of MAC-PHY interface
 * @return pointer to phy_device if a PHY is found, or NULL otherwise
 */
struct phy_device *phy_connect(struct mii_dev *bus, int addr,
				struct udevice *dev,
				phy_interface_t interface);

#else

/**
 * phy_connect_dev() - Associates the given pair of PHY and Ethernet devices
 * @phydev:	PHY device
 * @dev:	Ethernet device
 */
void phy_connect_dev(struct phy_device *phydev);

/**
 * phy_connect() - Creates a PHY device for the Ethernet interface
 *
 * Creates a PHY device for the PHY at the given address, if one doesn't exist
 * already, and associates it with the Ethernet device.
 * The function may be called with addr <= 0, in this case addr value is ignored
 * and the bus is scanned to detect a PHY.  Scanning should only be used if only
 * one PHY is expected to be present on the MDIO bus, otherwise it is undefined
 * which PHY is returned.
 *
 * @bus:	MII/MDIO bus that hosts the PHY
 * @addr:	PHY address on MDIO bus
 * @dev:	Ethernet device to associate to the PHY
 * @interface:	type of MAC-PHY interface
 * @return pointer to phy_device if a PHY is found, or NULL otherwise
 */
struct phy_device *phy_connect(struct mii_dev *bus, int addr, phy_interface_t interface);
#endif
int phy_startup(struct phy_device *phydev);
int phy_config(struct phy_device *phydev);
int phy_shutdown(struct phy_device *phydev);
int phy_set_supported(struct phy_device *phydev, uint32_t max_speed);
int genphy_config_aneg(struct phy_device *phydev);
int genphy_restart_aneg(struct phy_device *phydev);
int genphy_update_link(struct phy_device *phydev);
int genphy_parse_link(struct phy_device *phydev);
int genphy_config(struct phy_device *phydev);
int genphy_startup(struct phy_device *phydev);
int genphy_shutdown(struct phy_device *phydev);


int board_phy_config(struct phy_device *phydev);
int get_phy_id(struct mii_dev *bus, int addr, int devad, uint32_t *phy_id);

/**
 * phy_get_interface_by_name() - Look up a PHY interface name
 *
 * @str:	PHY interface name, e.g. "mii"
 * @return PHY_INTERFACE_MODE_... value, or -1 if not found
 */
int phy_get_interface_by_name(const char *str);

/**
 * phy_interface_is_rgmii - Convenience function for testing if a PHY interface
 * is RGMII (all variants)
 * @phydev: the phy_device struct
 */
static inline bool phy_interface_is_rgmii(struct phy_device *phydev)
{
	return phydev->interface >= PHY_INTERFACE_MODE_RGMII &&
		phydev->interface <= PHY_INTERFACE_MODE_RGMII_TXID;
}

/**
 * phy_interface_is_sgmii - Convenience function for testing if a PHY interface
 * is SGMII (all variants)
 * @phydev: the phy_device struct
 */
static inline bool phy_interface_is_sgmii(struct phy_device *phydev)
{
	return phydev->interface >= PHY_INTERFACE_MODE_SGMII &&
		phydev->interface <= PHY_INTERFACE_MODE_QSGMII;
}

/* PHY UIDs for various PHYs that are referenced in external code */
#define PHY_UID_CS4340  	0x13e51002
#define PHY_UID_CS4223  	0x03e57003
#define PHY_UID_TN2020		0x00a19410
#define PHY_UID_IN112525_S03	0x02107440

#endif
