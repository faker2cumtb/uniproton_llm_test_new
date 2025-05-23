#include <stdint.h>
#include "hal_base.h"
#include "errno2.h"
#include "bitops.h"
#include "hardware.h"
#include "grf_rk3588.h"
#include "rk_gmac.h"

static const struct eqos_config eqos_rockchip_config = {
    .reg_access_always_ok = false,
    .mdio_wait = 10000,
    .swr_wait = 200,
    .config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_NOT_ENABLED,
    .config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_100_150,
    .ops = &eqos_rockchip_ops,
};

static int rk3588_set_rgmii_speed(struct rk_gmac_platdata *pdata, struct eqos_priv *eqos)
{
    struct rk3588_php_grf *php_grf;
    unsigned int div, div_mask;

    enum
    {
        RK3588_GMAC_CLK_RGMII_DIV_SHIFT = 2,
        RK3588_GMAC_CLK_RGMII_DIV_MASK = GENMASK(3, 2),
        RK3588_GMAC_CLK_RGMII_DIV1 = 0,
        RK3588_GMAC_CLK_RGMII_DIV5 = GENMASK(3, 2),
        RK3588_GMAC_CLK_RGMII_DIV50 = BIT(3),
        RK3588_GMAC_CLK_RMII_DIV2 = BIT(2),
        RK3588_GMAC_CLK_RMII_DIV20 = 0,
        RK3588_GMAC1_ID_SHIFT = 5,
    };

    php_grf = PHP_GRF;
    switch (eqos->phy->speed)
    {
    case 10:
        if (eqos->interface == PHY_INTERFACE_MODE_RMII)
            div = RK3588_GMAC_CLK_RMII_DIV20;
        else
            div = RK3588_GMAC_CLK_RGMII_DIV50;
        break;
    case 100:
        if (eqos->interface == PHY_INTERFACE_MODE_RMII)
            div = RK3588_GMAC_CLK_RMII_DIV2;
        else
            div = RK3588_GMAC_CLK_RGMII_DIV5;
        break;
    case 1000:
        if (eqos->interface != PHY_INTERFACE_MODE_RMII)
            div = RK3588_GMAC_CLK_RGMII_DIV1;
        else
            return -EINVAL;
        break;
    default:
        HAL_DBG("Unknown phy speed: %d\n", eqos->phy->speed);
        return -EINVAL;
    }

    div <<= pdata->bus_id ? RK3588_GMAC1_ID_SHIFT : 0;
    div_mask = pdata->bus_id ? (RK3588_GMAC_CLK_RGMII_DIV_MASK << 5) : RK3588_GMAC_CLK_RGMII_DIV_MASK;

    rk_clrsetreg(&php_grf->clk_con1, div_mask, div);

    return 0;
}

static void rk3588_set_to_rmii(struct rk_gmac_platdata *pdata)
{
    unsigned int intf_sel, intf_sel_mask;
    unsigned int clk_mode, clk_mode_mask;
    struct rk3588_php_grf *php_grf = PHP_GRF;

    enum
    {
        RK3588_GMAC_PHY_INTF_SEL_SHIFT = 3,
        RK3588_GMAC_PHY_INTF_SEL_MASK = GENMASK(5, 3),
        RK3588_GMAC_PHY_INTF_SEL_RMII = BIT(5),
    };

    enum
    {
        RK3588_GMAC_CLK_RMII_MODE_SHIFT = 0x0,
        RK3588_GMAC_CLK_RMII_MODE_MASK = BIT(0),
        RK3588_GMAC_CLK_RMII_MODE = 0x1,
    };

    if (pdata->bus_id == 1)
    {
        intf_sel = RK3588_GMAC_PHY_INTF_SEL_RMII << 6;
        intf_sel_mask = RK3588_GMAC_PHY_INTF_SEL_MASK << 6;
        clk_mode = RK3588_GMAC_CLK_RMII_MODE << 5;
        clk_mode_mask = RK3588_GMAC_CLK_RMII_MODE_MASK << 5;
    }
    else
    {
        intf_sel = RK3588_GMAC_PHY_INTF_SEL_RMII;
        intf_sel_mask = RK3588_GMAC_PHY_INTF_SEL_MASK;
        clk_mode = RK3588_GMAC_CLK_RMII_MODE;
        clk_mode_mask = RK3588_GMAC_CLK_RMII_MODE_MASK;
    }

    rk_clrsetreg(&php_grf->gmac_con0, intf_sel_mask, intf_sel);
    rk_clrsetreg(&php_grf->clk_con1, clk_mode_mask, clk_mode);
}

static void rk3588_set_to_rgmii(struct rk_gmac_platdata *pdata)
{
    unsigned int rx_enable, rx_enable_mask, tx_enable, tx_enable_mask;
    unsigned int intf_sel, intf_sel_mask;
    unsigned int clk_mode, clk_mode_mask;
    unsigned int rx_delay;
    struct rk3588_php_grf *php_grf = PHP_GRF;
    struct rk3588_sys_grf *grf = SYS_GRF;
    void *offset_con;

    enum
    {
        RK3588_GMAC_PHY_INTF_SEL_SHIFT = 3,
        RK3588_GMAC_PHY_INTF_SEL_MASK = GENMASK(5, 3),
        RK3588_GMAC_PHY_INTF_SEL_RGMII = BIT(3),

        RK3588_RXCLK_DLY_ENA_GMAC_MASK = BIT(3),
        RK3588_RXCLK_DLY_ENA_GMAC_DISABLE = 0,
        RK3588_RXCLK_DLY_ENA_GMAC_ENABLE = BIT(3),

        RK3588_TXCLK_DLY_ENA_GMAC_MASK = BIT(2),
        RK3588_TXCLK_DLY_ENA_GMAC_DISABLE = 0,
        RK3588_TXCLK_DLY_ENA_GMAC_ENABLE = BIT(2),
    };

    enum
    {
        RK3588_CLK_RX_DL_CFG_GMAC_SHIFT = 0x8,
        RK3588_CLK_RX_DL_CFG_GMAC_MASK = GENMASK(15, 8),

        RK3588_CLK_TX_DL_CFG_GMAC_SHIFT = 0x0,
        RK3588_CLK_TX_DL_CFG_GMAC_MASK = GENMASK(7, 0),
    };

    enum
    {
        RK3588_GMAC_CLK_RGMII_MODE_SHIFT = 0x0,
        RK3588_GMAC_CLK_RGMII_MODE_MASK = BIT(0),
        RK3588_GMAC_CLK_RGMII_MODE = 0x0,
    };

    if (pdata->rx_delay < 0)
    {
        rx_enable = RK3588_RXCLK_DLY_ENA_GMAC_DISABLE;
        rx_delay = 0;
    }
    else
    {
        rx_enable = RK3588_RXCLK_DLY_ENA_GMAC_ENABLE;
        rx_delay = pdata->rx_delay << RK3588_CLK_RX_DL_CFG_GMAC_SHIFT;
    }

    if (pdata->bus_id == 1)
    {
        offset_con = &grf->soc_con9;
        rx_enable = rx_delay << 2;
        rx_enable_mask = RK3588_RXCLK_DLY_ENA_GMAC_MASK << 2;
        tx_enable = RK3588_TXCLK_DLY_ENA_GMAC_ENABLE << 2;
        tx_enable_mask = RK3588_TXCLK_DLY_ENA_GMAC_MASK << 2;
        intf_sel = RK3588_GMAC_PHY_INTF_SEL_RGMII << 6;
        intf_sel_mask = RK3588_GMAC_PHY_INTF_SEL_MASK << 6;
        clk_mode = RK3588_GMAC_CLK_RGMII_MODE << 5;
        clk_mode_mask = RK3588_GMAC_CLK_RGMII_MODE_MASK << 5;
    }
    else
    {
        offset_con = &grf->soc_con8;
        rx_enable_mask = RK3588_RXCLK_DLY_ENA_GMAC_MASK;
        tx_enable = RK3588_TXCLK_DLY_ENA_GMAC_ENABLE;
        tx_enable_mask = RK3588_TXCLK_DLY_ENA_GMAC_MASK;
        intf_sel = RK3588_GMAC_PHY_INTF_SEL_RGMII;
        intf_sel_mask = RK3588_GMAC_PHY_INTF_SEL_MASK;
        clk_mode = RK3588_GMAC_CLK_RGMII_MODE;
        clk_mode_mask = RK3588_GMAC_CLK_RGMII_MODE_MASK;
    }

    rk_clrsetreg(offset_con,
                 RK3588_CLK_TX_DL_CFG_GMAC_MASK |
                     RK3588_CLK_RX_DL_CFG_GMAC_MASK,
                 (pdata->tx_delay << RK3588_CLK_TX_DL_CFG_GMAC_SHIFT) |
                     rx_delay);

    rk_clrsetreg(&grf->soc_con7, tx_enable_mask | rx_enable_mask,
                 tx_enable | rx_enable);

    rk_clrsetreg(&php_grf->gmac_con0, intf_sel_mask, intf_sel);
    rk_clrsetreg(&php_grf->clk_con1, clk_mode_mask, clk_mode);
}

void gmac_set_rgmii(struct rk_gmac_dev *dev, uint32_t tx_delay, uint32_t rx_delay)
{
    struct rk_gmac_platdata *pdata = &dev->platdata;

    pdata->tx_delay = tx_delay;
    pdata->rx_delay = rx_delay;

    rk3588_set_to_rgmii(pdata);
}

static void rk3588_set_clock_selection(struct rk_gmac_platdata *pdata)
{
    struct rk3588_php_grf *php_grf = PHP_GRF;
    unsigned int val, mask;

    enum
    {
        RK3588_GMAC_CLK_SELET_SHIFT = 0x4,
        RK3588_GMAC_CLK_SELET_MASK = BIT(4),
        RK3588_GMAC_CLK_SELET_CRU = BIT(4),
        RK3588_GMAC_CLK_SELET_IO = 0,
    };

    val = pdata->clock_input ? RK3588_GMAC_CLK_SELET_IO : RK3588_GMAC_CLK_SELET_CRU;
    mask = RK3588_GMAC_CLK_SELET_MASK;

    if (pdata->bus_id == 1)
    {
        val <<= 5;
        mask <<= 5;
    }

    rk_clrsetreg(&php_grf->clk_con1, mask, val);
}

int rk_gmac_probe(struct rk_gmac_dev *dev)
{
    struct eqos_priv *eqos = &dev->eqos;
    struct rk_gmac_platdata *pdata = &dev->platdata;

    if (pdata->bus_id == 1)
    {
        eqos->regs = GMAC1_BASE;
    }
    else
    {
        eqos->regs = GMAC0_BASE;
    }

    eqos->config = &eqos_rockchip_config;
    rk3588_set_clock_selection(pdata);

    switch (eqos->interface)
    {
    case PHY_INTERFACE_MODE_RGMII:
    case PHY_INTERFACE_MODE_RGMII_RXID:
        if (eqos->interface == PHY_INTERFACE_MODE_RGMII_RXID)
        {
            pdata->rx_delay = -1;
        }

        rk3588_set_to_rgmii(pdata);

        break;
    case PHY_INTERFACE_MODE_RMII:
        rk3588_set_to_rmii(pdata);

        break;
    default:
        HAL_DBG("NO interface defined!\n");
        return -ENXIO;
    }

    return eqos_probe(eqos);
}

int rk_gmac_start(struct rk_gmac_dev *dev)
{
    int ret = eqos_init(&dev->eqos);
    if (ret)
        return ret;
        
    ret = rk3588_set_rgmii_speed(&dev->platdata, &dev->eqos);
    if (ret)
        return ret;

    eqos_enable(&dev->eqos);
    return 0;
}

void rk_gmac_stop(struct rk_gmac_dev *dev)
{
    eqos_stop(&dev->eqos);
}