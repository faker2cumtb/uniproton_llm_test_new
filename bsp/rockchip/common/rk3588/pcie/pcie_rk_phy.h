/*
 * Rockchip PCIE3.0 phy driver
 *
 * Copyright (C) 2024 NCTI
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PCIE_RK_PHY_H__
#define __PCIE_RK_PHY_H__

#define PHY_MODE_PCIE_INVALID 		-1
#define PHY_MODE_PCIE_RC		 	20
#define PHY_MODE_PCIE_EP			21
#define PHY_MODE_PCIE_BIFURCATION 	22

struct rk_pcie3_phy_cfg{
	bool 					is_bifurcation;
	int						phy_mode;
};

struct rk_pcie3_phy_priv {
	uintptr_t reg;
	int mode;
	uintptr_t grf;
	size_t grf_size;
	size_t reg_size;
};

struct rk_pcie3_phy_data {
	uintptr_t 				reg_mp;
	uintptr_t 				grf_mp;
	uintptr_t 				cru_mp;
	struct rk_pcie3_phy_cfg	phy_cfg;
};

struct rk_pcie3_cfg {
	uintptr_t 				phy_grf;
	uintptr_t 				pipe_grf;
	uintptr_t 				cru;
	uintptr_t 		 		iobus;
	uintptr_t apbBase;
    uintptr_t dbiBase;
};

static inline void reg_write16(uint32_t value, uintptr_t address)
{
    ((*(volatile uint16_t *)(address)) = (value));
}

static inline void reg_write32(uint32_t value, uintptr_t address)
{
    (*(volatile uint32_t *)((uintptr_t)(address)) = (uint32_t)(value));
}

static inline uint32_t reg_read32(uintptr_t address)
{
    return (*(volatile uint32_t *)((uintptr_t)(address)));
}

static inline uintptr_t reg_read64(uintptr_t address)
{
    return (*(volatile uintptr_t *)((uintptr_t)(address)));
}

static inline void reg_write64(uint32_t value, uintptr_t address)
{
    (*(volatile uintptr_t *)((uintptr_t)(address)) = (uint32_t)(value));
}

int rk_p3phy_init(struct rk_pcie3_cfg *dw_pcie_cfg);

#endif