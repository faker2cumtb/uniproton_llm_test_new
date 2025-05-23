#include <stdint.h>
#include "prt_clk.h"
#include "errno.h"
#include "bitops.h"
#include "hardware.h"
#include "pcie_rk_phy.h"
#include "hal_def.h"
#include "hal_base.h"
#include "print.h"

/* Register for RK3588 */
#define PHP_GRF_PCIESEL_CON 				0x100
#define RK3588_PCIE3PHY_GRF_CMN_CON0		0x0
#define RK3588_PCIE3PHY_GRF_PHY0_STATUS1 	0x904
#define RK3588_PCIE3PHY_GRF_PHY1_STATUS1 	0xa04

/*
 * pcie30_phy_mode[2:0]
 * bit2: aggregation
 * bit1: bifurcation for port 1
 * bit0: bifurcation for port 0
 */
#define PHY_MODE_PCIE_AGGREGATION 			4     /* PCIe3x4 */
#define PHY_MODE_PCIE_NANBNB    			0       /* P1:PCIe3x2  +  P0:PCIe3x2 */
#define PHY_MODE_PCIE_NANBBI    			1       /* P1:PCIe3x2  +  P0:PCIe3x1*2 */
#define PHY_MODE_PCIE_NABINB    			2       /* P1:PCIe3x1*2 + P0:PCIe3x2 */
#define PHY_MODE_PCIE_NABIBI    			3       /* P1:PCIe3x1*2 + P0:PCIe3x1*2 */

uint32_t regmap_read(uintptr_t addr, uint32_t offset, uint32_t *valp)
{
	*valp = reg_read32(addr + offset);

	return 0;
}

uint32_t regmap_write(uintptr_t addr, uint32_t offset, uint32_t val)
{
	reg_write32(val, addr + offset);

	return 0;
}

int rk_p3phy_init(struct rk_pcie3_cfg *dw_pcie_cfg)
{

	uint32_t reg;
	uint32_t timeout;
	int pcie30_phymode = PHY_MODE_PCIE_AGGREGATION;

	/* Select correct pcie30_phymode */
	if (pcie30_phymode > 4)
		pcie30_phymode = PHY_MODE_PCIE_AGGREGATION;

	regmap_write(dw_pcie_cfg->phy_grf, RK3588_PCIE3PHY_GRF_CMN_CON0,
		     (0x7<<16) | pcie30_phymode);

	/* Set pcie1ln_sel in PHP_GRF_PCIESEL_CON */
	reg = pcie30_phymode & 3;
	if (reg)
		regmap_write(dw_pcie_cfg->pipe_grf, PHP_GRF_PCIESEL_CON,
			     (reg << 16) | reg);

	timeout = 500;
	while (timeout--) {
		regmap_read(dw_pcie_cfg->phy_grf, RK3588_PCIE3PHY_GRF_PHY0_STATUS1, &reg);
		if (reg & 0x1)
			break;
		HAL_CPUDelayUs(1);
	}

	if (timeout <= 0) {
		PRT_Printf("%s: phy0 lock failed, check input refclk and power supply\n", __func__);
		return -ETIMEDOUT;
	}

	timeout = 500;
	while (timeout--) {
		regmap_read(dw_pcie_cfg->phy_grf, RK3588_PCIE3PHY_GRF_PHY1_STATUS1, &reg);
		if (reg & 0x1)
			break;
		HAL_CPUDelayUs(1);
	}

	if (timeout <= 0) {
		PRT_Printf("%s: phy1 lock failed, check input refclk and power supply\n", __func__);
		return -ETIMEDOUT;
	}

	/* Deassert PCIe PMA output clamp mode */
	regmap_write(dw_pcie_cfg->phy_grf, RK3588_PCIE3PHY_GRF_CMN_CON0,
		     (0x1 << 8) | (0x1 << 24));

	return 0;
}
