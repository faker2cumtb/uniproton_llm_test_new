/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * Portions based on U-Boot's rtl8169.c.
 */

/*
 * This driver supports the Synopsys Designware Ethernet QOS (Quality Of
 * Service) IP block. The IP supports multiple options for bus type, clocking/
 * reset structure, and feature list.
 *
 * The driver is written such that generic core logic is kept separate from
 * configuration-specific logic. Code that interacts with configuration-
 * specific resources is split out into separate functions to avoid polluting
 * common code. If/when this driver is enhanced to support multiple
 * configurations, the core code should be adapted to call all configuration-
 * specific functions through function pointers, with the definition of those
 * function pointers being supplied by struct udevice_id eqos_ids[]'s .data
 * field.
 */

#include <prt_task.h>
#include <prt_cache_external.h>
#include "errno2.h"
#include "bitops.h"
#include "hardware.h"
#include "dwc_eth_qos.h"

/* Core registers */

#define EQOS_MAC_REGS_BASE 0x000
struct eqos_mac_regs
{
    uint32_t configuration;                   /* 0x000 */
    uint32_t unused_004[(0x070 - 0x004) / 4]; /* 0x004 */
    uint32_t q0_tx_flow_ctrl;                 /* 0x070 */
    uint32_t unused_070[(0x090 - 0x074) / 4]; /* 0x074 */
    uint32_t rx_flow_ctrl;                    /* 0x090 */
    uint32_t unused_094;                      /* 0x094 */
    uint32_t txq_prty_map0;                   /* 0x098 */
    uint32_t unused_09c;                      /* 0x09c */
    uint32_t rxq_ctrl0;                       /* 0x0a0 */
    uint32_t unused_0a4;                      /* 0x0a4 */
    uint32_t rxq_ctrl2;                       /* 0x0a8 */
    uint32_t unused_0ac[(0x0dc - 0x0ac) / 4]; /* 0x0ac */
    uint32_t us_tic_counter;                  /* 0x0dc */
    uint32_t unused_0e0[(0x11c - 0x0e0) / 4]; /* 0x0e0 */
    uint32_t hw_feature0;                     /* 0x11c */
    uint32_t hw_feature1;                     /* 0x120 */
    uint32_t hw_feature2;                     /* 0x124 */
    uint32_t unused_128[(0x200 - 0x128) / 4]; /* 0x128 */
    uint32_t mdio_address;                    /* 0x200 */
    uint32_t mdio_data;                       /* 0x204 */
    uint32_t unused_208[(0x300 - 0x208) / 4]; /* 0x208 */
    uint32_t address0_high;                   /* 0x300 */
    uint32_t address0_low;                    /* 0x304 */
};

#define EQOS_MAC_CONFIGURATION_GPSLCE BIT(23)
#define EQOS_MAC_CONFIGURATION_CST BIT(21)
#define EQOS_MAC_CONFIGURATION_ACS BIT(20)
#define EQOS_MAC_CONFIGURATION_WD BIT(19)
#define EQOS_MAC_CONFIGURATION_JD BIT(17)
#define EQOS_MAC_CONFIGURATION_JE BIT(16)
#define EQOS_MAC_CONFIGURATION_PS BIT(15)
#define EQOS_MAC_CONFIGURATION_FES BIT(14)
#define EQOS_MAC_CONFIGURATION_DM BIT(13)
#define EQOS_MAC_CONFIGURATION_LM BIT(12)
#define EQOS_MAC_CONFIGURATION_TE BIT(1)
#define EQOS_MAC_CONFIGURATION_RE BIT(0)

#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT 16
#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_MASK 0xffff
#define EQOS_MAC_Q0_TX_FLOW_CTRL_TFE BIT(1)

#define EQOS_MAC_RX_FLOW_CTRL_RFE BIT(0)

#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT 0
#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK 0xff

#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT 0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK 3

#define EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT 0
#define EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK 0xff

#define EQOS_MAC_HW_FEATURE0_MMCSEL_SHIFT 8
#define EQOS_MAC_HW_FEATURE0_HDSEL_SHIFT 2
#define EQOS_MAC_HW_FEATURE0_GMIISEL_SHIFT 1
#define EQOS_MAC_HW_FEATURE0_MIISEL_SHIFT 0

#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT 6
#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK 0x1f
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT 0
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK 0x1f

#define EQOS_MAC_HW_FEATURE3_ASP_SHIFT 28
#define EQOS_MAC_HW_FEATURE3_ASP_MASK 0x3

#define EQOS_MAC_MDIO_ADDRESS_PA_SHIFT 21
#define EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT 16
#define EQOS_MAC_MDIO_ADDRESS_CR_SHIFT 8
#define EQOS_MAC_MDIO_ADDRESS_SKAP BIT(4)
#define EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT 2
#define EQOS_MAC_MDIO_ADDRESS_GOC_READ 3
#define EQOS_MAC_MDIO_ADDRESS_GOC_WRITE 1
#define EQOS_MAC_MDIO_ADDRESS_C45E BIT(1)
#define EQOS_MAC_MDIO_ADDRESS_GB BIT(0)

#define EQOS_MAC_MDIO_DATA_GD_MASK 0xffff

#define EQOS_MTL_REGS_BASE 0xd00
struct eqos_mtl_regs
{
    uint32_t txq0_operation_mode;             /* 0xd00 */
    uint32_t unused_d04;                      /* 0xd04 */
    uint32_t txq0_debug;                      /* 0xd08 */
    uint32_t unused_d0c[(0xd18 - 0xd0c) / 4]; /* 0xd0c */
    uint32_t txq0_quantum_weight;             /* 0xd18 */
    uint32_t unused_d1c[(0xd30 - 0xd1c) / 4]; /* 0xd1c */
    uint32_t rxq0_operation_mode;             /* 0xd30 */
    uint32_t unused_d34;                      /* 0xd34 */
    uint32_t rxq0_debug;                      /* 0xd38 */
};

#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT 16
#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK 0x1ff
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT 2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_MASK 3
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED 2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TSF BIT(1)
#define EQOS_MTL_TXQ0_OPERATION_MODE_FTQ BIT(0)

#define EQOS_MTL_TXQ0_DEBUG_TXQSTS BIT(4)
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT 1
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK 3

#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT 20
#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK 0x3ff
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT 14
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK 0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT 8
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK 0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_EHFC BIT(7)
#define EQOS_MTL_RXQ0_OPERATION_MODE_RSF BIT(5)
#define EQOS_MTL_RXQ0_OPERATION_MODE_FEP BIT(4)
#define EQOS_MTL_RXQ0_OPERATION_MODE_FUP BIT(3)

#define EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT 16
#define EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK 0x7fff
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT 4
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK 3

#define EQOS_DMA_REGS_BASE 0x1000
struct eqos_dma_regs
{
    uint32_t mode;                               /* 0x1000 */
    uint32_t sysbus_mode;                        /* 0x1004 */
    uint32_t unused_1008[(0x1100 - 0x1008) / 4]; /* 0x1008 */
    uint32_t ch0_control;                        /* 0x1100 */
    uint32_t ch0_tx_control;                     /* 0x1104 */
    uint32_t ch0_rx_control;                     /* 0x1108 */
    uint32_t unused_110c;                        /* 0x110c */
    uint32_t ch0_txdesc_list_haddress;           /* 0x1110 */
    uint32_t ch0_txdesc_list_address;            /* 0x1114 */
    uint32_t ch0_rxdesc_list_haddress;           /* 0x1118 */
    uint32_t ch0_rxdesc_list_address;            /* 0x111c */
    uint32_t ch0_txdesc_tail_pointer;            /* 0x1120 */
    uint32_t unused_1124;                        /* 0x1124 */
    uint32_t ch0_rxdesc_tail_pointer;            /* 0x1128 */
    uint32_t ch0_txdesc_ring_length;             /* 0x112c */
    uint32_t ch0_rxdesc_ring_length;             /* 0x1130 */
    uint32_t ch0_interrupt_enable;               /* 0x1134 */
    uint32_t ch0_rx_interrupt_wd_timer;          /* 0x1138 */
    uint32_t ch0_slot_func_ctrl_status;          /* 0x113c */
    uint32_t unused_1140[1];                     /* 0x1140 */
    uint32_t ch0_current_app_txdesc;             /* 0x1144 */
    uint32_t unused_1148[1];                     /* 0x1148 */
    uint32_t ch0_current_app_rxdesc;             /* 0x114c */
    uint32_t unused_1150[1];                     /* 0x1150 */
    uint32_t ch0_current_app_txbuffer;           /* 0x1154 */
    uint32_t unused_1158[1];                     /* 0x1158 */
    uint32_t ch0_current_app_rxbuffer;           /* 0x115c */
    uint32_t ch0_status;                         /* 0x1160 */
    uint32_t ch0_miss_frame_cnt;                 /* 0x1164 */
    uint32_t ch0_rx_eri_cnt;                     /* 0x1168 */
};

#define EQOS_DMA_MODE_SWR BIT(0)

#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT 16
#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_MASK 0xf
#define EQOS_DMA_SYSBUS_MODE_EAME BIT(11)
#define EQOS_DMA_SYSBUS_MODE_BLEN16 BIT(3)
#define EQOS_DMA_SYSBUS_MODE_BLEN8 BIT(2)
#define EQOS_DMA_SYSBUS_MODE_BLEN4 BIT(1)

#define EQOS_DMA_CH0_CONTROL_PBLX8 BIT(16)

#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT 16
#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK 0x3f
#define EQOS_DMA_CH0_TX_CONTROL_OSP BIT(4)
#define EQOS_DMA_CH0_TX_CONTROL_ST BIT(0)

#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT 16
#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK 0x3f
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT 1
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK 0x3fff
#define EQOS_DMA_CH0_RX_CONTROL_SR BIT(0)

#define EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD BIT(31)

#define EQOS_AUTO_CAL_CONFIG_START BIT(31)
#define EQOS_AUTO_CAL_CONFIG_ENABLE BIT(29)

#define EQOS_AUTO_CAL_STATUS_ACTIVE BIT(31)

/* Descriptors */

#define EQOS_DESCRIPTOR_WORDS 4
#define EQOS_DESCRIPTOR_SIZE (EQOS_DESCRIPTOR_WORDS * 4)
/* We assume ARCH_DMA_MINALIGN >= 16; 16 is the EQOS HW minimum */
#define EQOS_DESCRIPTOR_ALIGN ARCH_DMA_MINALIGN
#define EQOS_DESCRIPTORS_TX 16
#define EQOS_DESCRIPTORS_RX 16
#define EQOS_DESCRIPTORS_NUM (EQOS_DESCRIPTORS_TX + EQOS_DESCRIPTORS_RX)
#define EQOS_DESCRIPTORS_SIZE ALIGN(EQOS_DESCRIPTORS_NUM *    \
                                        EQOS_DESCRIPTOR_SIZE, \
                                    ARCH_DMA_MINALIGN)
#define EQOS_BUFFER_ALIGN ARCH_DMA_MINALIGN
#define EQOS_MAX_PACKET_SIZE ALIGN(1568, ARCH_DMA_MINALIGN)
#define EQOS_RX_BUFFER_SIZE (EQOS_DESCRIPTORS_RX * EQOS_MAX_PACKET_SIZE)

struct eqos_desc
{
    uint32_t des0;
    uint32_t des1;
    uint32_t des2;
    uint32_t des3;
};

#define EQOS_DESC2_IOC BIT(31)

#define EQOS_DESC3_OWN BIT(31)
#define EQOS_DESC3_IOC BIT(30)
#define EQOS_DESC3_FD BIT(29)
#define EQOS_DESC3_LD BIT(28)
#define EQOS_DESC3_BUF1V BIT(24)

/* The forced speed, 10Mb, 100Mb, gigabit, 2.5Gb, 10GbE. */
#define SPEED_10 10
#define SPEED_100 100
#define SPEED_1000 1000
#define SPEED_2500 2500
#define SPEED_10000 10000

/*
 * TX and RX descriptors are 16 bytes. This causes problems with the cache
 * maintenance on CPUs where the cache-line size exceeds the size of these
 * descriptors. What will happen is that when the driver receives a packet
 * it will be immediately requeued for the hardware to reuse. The CPU will
 * therefore need to flush the cache-line containing the descriptor, which
 * will cause all other descriptors in the same cache-line to be flushed
 * along with it. If one of those descriptors had been written to by the
 * device those changes (and the associated packet) will be lost.
 *
 * To work around this, we make use of non-cached memory if available. If
 * descriptors are mapped uncached there's no need to manually flush them
 * or invalidate them.
 *
 * Note that this only applies to descriptors. The packet data buffers do
 * not have the same constraints since they are 1536 bytes large, so they
 * are unlikely to share cache-lines.
 */

static void eqos_inval_desc_generic(void *desc)
{
    // 描述符不需要刷新cache
}

static void eqos_flush_desc_generic(void *desc)
{
    // 描述符不需要刷新cache
}

extern void cache_flush_by_range(void *addr, size_t size);
extern void cache_invalid_by_range(void *addr, size_t size);
static void eqos_inval_buffer_generic(void *buf, size_t size)
{
    cache_invalid_by_range(buf, size);
    //os_asm_invalidate_dcache_all();
}

static void eqos_flush_buffer_generic(void *buf, size_t size)
{
    cache_flush_by_range(buf, size);
    //os_asm_flush_dcache_all();
}

static int eqos_mdio_wait_idle(struct eqos_priv *eqos)
{
    uint32_t delay = 0;

    while ((readl(&eqos->mac_regs->mdio_address) & EQOS_MAC_MDIO_ADDRESS_GB) == EQOS_MAC_MDIO_ADDRESS_GB)
    {
        /* Check for the Timeout */
        if (++delay > 100)
        {
            return HAL_TIMEOUT;
        }
        HAL_DelayMs(1);
    }

    return HAL_OK;
}

static int eqos_mdio_read(struct mii_dev *bus, int mdio_addr, int mdio_devad,
                          int mdio_reg)
{
    struct eqos_priv *eqos = bus->priv;
    uint32_t val;
    int ret;

    // HAL_DBG("%s(addr=%#x, reg=%d):\n", __func__, mdio_addr, mdio_reg);

    ret = eqos_mdio_wait_idle(eqos);
    if (ret)
    {
        HAL_DBG_ERR("MDIO not idle at entry\n");
        return ret;
    }

    val = readl(&eqos->mac_regs->mdio_address);
    val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
           EQOS_MAC_MDIO_ADDRESS_C45E;
    val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
           (mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
           (eqos->config->config_mac_mdio << EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
           (EQOS_MAC_MDIO_ADDRESS_GOC_READ << EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
           EQOS_MAC_MDIO_ADDRESS_GB;
    writel(val, &eqos->mac_regs->mdio_address);

    HAL_DelayUs(eqos->config->mdio_wait);

    ret = eqos_mdio_wait_idle(eqos);
    if (ret)
    {
        HAL_DBG_ERR("MDIO read didn't complete\n");
        return ret;
    }

    val = readl(&eqos->mac_regs->mdio_data);
    val &= EQOS_MAC_MDIO_DATA_GD_MASK;

    // HAL_DBG("%s: val=%x\n", __func__, val);

    return val;
}

static int eqos_mdio_write(struct mii_dev *bus, int mdio_addr, int mdio_devad,
                           int mdio_reg, uint16_t mdio_val)
{
    struct eqos_priv *eqos = bus->priv;
    uint32_t val;
    int ret;

    // HAL_DBG("%s(addr=%#x, reg=%d, val=%#x):\n", __func__, mdio_addr, mdio_reg, mdio_val);

    ret = eqos_mdio_wait_idle(eqos);
    if (ret)
    {
        HAL_DBG_ERR("MDIO not idle at entry\n");
        return ret;
    }

    writel(mdio_val, &eqos->mac_regs->mdio_data);

    val = readl(&eqos->mac_regs->mdio_address);
    val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
           EQOS_MAC_MDIO_ADDRESS_C45E;
    val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
           (mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
           (eqos->config->config_mac_mdio << EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
           (EQOS_MAC_MDIO_ADDRESS_GOC_WRITE << EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
           EQOS_MAC_MDIO_ADDRESS_GB;
    writel(val, &eqos->mac_regs->mdio_address);

    HAL_DelayUs(eqos->config->mdio_wait);

    ret = eqos_mdio_wait_idle(eqos);
    if (ret)
    {
        HAL_DBG_ERR("MDIO read didn't complete\n");
        return ret;
    }

    return 0;
}

static int eqos_start_resets_stm32(struct eqos_priv *eqos)
{
    return 0;
}

static int eqos_stop_resets_stm32(struct eqos_priv *eqos)
{
    return 0;
}

static ulong eqos_get_tick_clk_rate_stm32(struct eqos_priv *eqos)
{
    /* 暂不处理 */
    return 0;
}

static int eqos_calibrate_pads_stm32(struct eqos_priv *eqos)
{
    return 0;
}

static int eqos_disable_calibration_stm32(struct eqos_priv *eqos)
{
    return 0;
}

static int eqos_set_full_duplex(struct eqos_priv *eqos)
{
    HAL_DBG("%s\n", __func__);

    SET_BIT(eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

    return 0;
}

static int eqos_set_half_duplex(struct eqos_priv *eqos)
{
    HAL_DBG("%s\n", __func__);

    CLEAR_BIT(eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

    /* WAR: Flush TX queue when switching to half-duplex */
    SET_BIT(eqos->mtl_regs->txq0_operation_mode,
            EQOS_MTL_TXQ0_OPERATION_MODE_FTQ);

    return 0;
}

static int eqos_set_gmii_speed(struct eqos_priv *eqos)
{
    HAL_DBG("%s\n", __func__);

    CLEAR_BIT(eqos->mac_regs->configuration,
              EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

    return 0;
}

static int eqos_set_mii_speed_100(struct eqos_priv *eqos)
{
    HAL_DBG("%s\n", __func__);

    SET_BIT(eqos->mac_regs->configuration,
            EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

    return 0;
}

static int eqos_set_mii_speed_10(struct eqos_priv *eqos)
{
    HAL_DBG("%s\n", __func__);

    MODIFY_REG(eqos->mac_regs->configuration,
               EQOS_MAC_CONFIGURATION_FES, EQOS_MAC_CONFIGURATION_PS);

    return 0;
}

static int eqos_set_tx_clk_speed_stm32(struct eqos_priv *eqos)
{
    return 0;
}

static int eqos_adjust_link(struct eqos_priv *eqos)
{
    int ret;
    bool en_calibration;

    HAL_DBG("%s\n", __func__);

    if (eqos->phy->duplex)
        ret = eqos_set_full_duplex(eqos);
    else
        ret = eqos_set_half_duplex(eqos);
    if (ret < 0)
    {
        HAL_DBG_ERR("eqos_set_*_duplex() failed: %d\n", ret);
        return ret;
    }

    switch (eqos->phy->speed)
    {
    case SPEED_1000:
        en_calibration = true;
        ret = eqos_set_gmii_speed(eqos);
        break;
    case SPEED_100:
        en_calibration = true;
        ret = eqos_set_mii_speed_100(eqos);
        break;
    case SPEED_10:
        en_calibration = false;
        ret = eqos_set_mii_speed_10(eqos);
        break;
    default:
        HAL_DBG_ERR("invalid speed %d\n", eqos->phy->speed);
        return -EINVAL;
    }
    if (ret < 0)
    {
        HAL_DBG_ERR("eqos_set_*mii_speed*() failed: %d\n", ret);
        return ret;
    }

    if (en_calibration)
    {
        ret = eqos->config->ops->eqos_calibrate_pads(eqos);
        if (ret < 0)
        {
            HAL_DBG_ERR("eqos_calibrate_pads() failed: %d\n",
                        ret);
            return ret;
        }
    }
    else
    {
        ret = eqos->config->ops->eqos_disable_calibration(eqos);
        if (ret < 0)
        {
            HAL_DBG_ERR("eqos_disable_calibration() failed: %d\n",
                        ret);
            return ret;
        }
    }
    ret = eqos->config->ops->eqos_set_tx_clk_speed(eqos);
    if (ret < 0)
    {
        HAL_DBG_ERR("eqos_set_tx_clk_speed() failed: %d\n", ret);
        return ret;
    }

    return 0;
}

int eqos_write_hwaddr(struct eqos_priv *eqos)
{
    uint32_t val;

    /*
     * This function may be called before start() or after stop(). At that
     * time, on at least some configurations of the EQoS HW, all clocks to
     * the EQoS HW block will be stopped, and a reset signal applied. If
     * any register access is attempted in this state, bus timeouts or CPU
     * hangs may occur. This check prevents that.
     *
     * A simple solution to this problem would be to not implement
     * write_hwaddr(), since start() always writes the MAC address into HW
     * anyway. However, it is desirable to implement write_hwaddr() to
     * support the case of SW that runs subsequent to U-Boot which expects
     * the MAC address to already be programmed into the EQoS registers,
     * which must happen irrespective of whether the U-Boot user (or
     * scripts) actually made use of the EQoS device, and hence
     * irrespective of whether start() was ever called.
     *
     * Note that this requirement by subsequent SW is not valid for
     * Tegra186, and is likely not valid for any non-PCI instantiation of
     * the EQoS HW block. This function is implemented solely as
     * future-proofing with the expectation the driver will eventually be
     * ported to some system where the expectation above is true.
     */
    if (!eqos->config->reg_access_always_ok && !eqos->reg_access_ok)
        return 0;

    /* Update the MAC address */
    val = (eqos->mac_addr[5] << 8) |
          (eqos->mac_addr[4]);
    writel(val, &eqos->mac_regs->address0_high);
    val = (eqos->mac_addr[3] << 24) |
          (eqos->mac_addr[2] << 16) |
          (eqos->mac_addr[1] << 8) |
          (eqos->mac_addr[0]);
    writel(val, &eqos->mac_regs->address0_low);

    return 0;
}

int eqos_init(struct eqos_priv *eqos)
{
    int ret = 0, limit = 10;
    ulong rate;
    uint32_t val;

    HAL_DBG("%s\n", __func__);

    if (eqos->config->ops->eqos_start_clks)
    {
        ret = eqos->config->ops->eqos_start_clks(eqos);
        if (ret < 0)
        {
            HAL_DBG_ERR("eqos_start_clks() failed: %d\n", ret);
            goto err;
        }
    }

    if (!eqos->mii_reseted)
    {
        ret = eqos->config->ops->eqos_start_resets(eqos);
        if (ret < 0)
        {
            HAL_DBG_ERR("eqos_start_resets() failed: %d\n", ret);
            goto err_stop_clks;
        }

        eqos->mii_reseted = true;
        HAL_DelayUs(10);
    }

    eqos->reg_access_ok = true;

    /* DMA SW reset */
    val = readl(&eqos->dma_regs->mode);
    val |= EQOS_DMA_MODE_SWR;
    writel(val, &eqos->dma_regs->mode);
    while (limit--)
    {
        if (!(readl(&eqos->dma_regs->mode) & EQOS_DMA_MODE_SWR))
            break;
        HAL_DelayMs(10);
    }

    if (limit < 0)
    {
        HAL_DBG_ERR("EQOS_DMA_MODE_SWR stuck\n");
        ret = -EAGAIN;
        goto err_stop_resets;
    }

    ret = eqos->config->ops->eqos_calibrate_pads(eqos);
    if (ret < 0)
    {
        HAL_DBG_ERR("eqos_calibrate_pads() failed: %d\n", ret);
        goto err_stop_resets;
    }
    rate = eqos->config->ops->eqos_get_tick_clk_rate(eqos);

    val = (rate / 1000000) - 1;
    writel(val, &eqos->mac_regs->us_tic_counter);

    /*
     * if PHY was already connected and configured,
     * don't need to reconnect/reconfigure again
     */
    if (!eqos->phy)
    {
        eqos->phy = phy_connect(eqos->mii, eqos->phy_addr, eqos->config->ops->eqos_get_interface(eqos));
        if (!eqos->phy)
        {
            HAL_DBG_ERR("phy_connect() failed\n");
            ret = -ENODEV;
            goto err_stop_resets;
        }

        if (eqos->max_speed)
        {
            ret = phy_set_supported(eqos->phy, eqos->max_speed);
            if (ret)
            {
                HAL_DBG_ERR("phy_set_supported() failed: %d\n", ret);
                goto err_shutdown_phy;
            }
        }

        ret = phy_config(eqos->phy);
        if (ret < 0)
        {
            HAL_DBG_ERR("phy_config() failed: %d\n", ret);
            goto err_shutdown_phy;
        }
    }

    ret = phy_startup(eqos->phy);
    if (ret < 0)
    {
        HAL_DBG_ERR("phy_startup() failed: %d\n", ret);
        goto err_shutdown_phy;
    }

    if (!eqos->phy->link)
    {
        HAL_DBG_ERR("No link\n");
        ret = -EINVAL;
        goto err_shutdown_phy;
    }

    ret = eqos_adjust_link(eqos);
    if (ret < 0)
    {
        HAL_DBG_ERR("eqos_adjust_link() failed: %d\n", ret);
        goto err_shutdown_phy;
    }

    HAL_DBG("%s: OK\n", __func__);
    return 0;

err_shutdown_phy:
    phy_shutdown(eqos->phy);
err_stop_resets:
    eqos->config->ops->eqos_stop_resets(eqos);
    eqos->mii_reseted = false;
err_stop_clks:
    if (eqos->config->ops->eqos_stop_clks)
        eqos->config->ops->eqos_stop_clks(eqos);
err:
    HAL_DBG_ERR("FAILED: %d\n", ret);
    return ret;
}

void eqos_enable(struct eqos_priv *eqos)
{
    uint32_t val, tx_fifo_sz, rx_fifo_sz, tqs, rqs, pbl;
    ulong last_rx_desc;
    int i;

    eqos->tx_desc_idx = 0;
    eqos->rx_desc_idx = 0;

    /* Configure MTL */
    writel(0x60, &eqos->mtl_regs->txq0_quantum_weight - 0x100);

    /* Enable Store and Forward mode for TX */
    /* Program Tx operating mode */
    SET_BIT(eqos->mtl_regs->txq0_operation_mode,
            EQOS_MTL_TXQ0_OPERATION_MODE_TSF |
                (EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED << EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT));

    /* Transmit Queue weight */
    writel(0x10, &eqos->mtl_regs->txq0_quantum_weight);

    /* Enable Store and Forward mode for RX, since no jumbo frame */
    SET_BIT(eqos->mtl_regs->rxq0_operation_mode,
            EQOS_MTL_RXQ0_OPERATION_MODE_RSF |
                EQOS_MTL_RXQ0_OPERATION_MODE_FEP |
                EQOS_MTL_RXQ0_OPERATION_MODE_FUP);

    /* Transmit/Receive queue fifo size; use all RAM for 1 queue */
    val = readl(&eqos->mac_regs->hw_feature1);
    tx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT) &
                 EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK;
    rx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT) &
                 EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK;

    /*
     * r/tx_fifo_sz is encoded as log2(n / 128). Undo that by shifting.
     * r/tqs is encoded as (n / 256) - 1.
     */
    tqs = (128 << tx_fifo_sz) / 256 - 1;
    rqs = (128 << rx_fifo_sz) / 256 - 1;

    MODIFY_REG(eqos->mtl_regs->txq0_operation_mode,
               EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK << EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT,
               tqs << EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
    MODIFY_REG(eqos->mtl_regs->rxq0_operation_mode,
               EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK << EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT,
               rqs << EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT);

    /* Flow control used only if each channel gets 4KB or more FIFO */
    if (rqs >= ((4096 / 256) - 1))
    {
        uint32_t rfd, rfa;

        SET_BIT(eqos->mtl_regs->rxq0_operation_mode,
                EQOS_MTL_RXQ0_OPERATION_MODE_EHFC);

        /*
         * Set Threshold for Activating Flow Contol space for min 2
         * frames ie, (1500 * 1) = 1500 bytes.
         *
         * Set Threshold for Deactivating Flow Contol for space of
         * min 1 frame (frame size 1500bytes) in receive fifo
         */
        if (rqs == ((4096 / 256) - 1))
        {
            /*
             * This violates the above formula because of FIFO size
             * limit therefore overflow may occur inspite of this.
             */
            rfd = 0x3; /* Full-3K */
            rfa = 0x1; /* Full-1.5K */
        }
        else if (rqs == ((8192 / 256) - 1))
        {
            rfd = 0x6; /* Full-4K */
            rfa = 0xa; /* Full-6K */
        }
        else if (rqs == ((16384 / 256) - 1))
        {
            rfd = 0x6;  /* Full-4K */
            rfa = 0x12; /* Full-10K */
        }
        else
        {
            rfd = 0x6;  /* Full-4K */
            rfa = 0x1E; /* Full-16K */
        }

        MODIFY_REG(eqos->mtl_regs->rxq0_operation_mode,
                   (EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK << EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
                       (EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK << EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT),
                   (rfd << EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
                       (rfa << EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT));
    }

    /* Configure MAC */

    MODIFY_REG(eqos->mac_regs->rxq_ctrl0,
               EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK << EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT,
               eqos->config->config_mac << EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

    MODIFY_REG(eqos->mac_regs->rxq_ctrl0,
               EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK << EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT,
               0x2 << EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

    /* Multicast and Broadcast Queue Enable */
    SET_BIT(eqos->mac_regs->unused_0a4,
            0x00100000);
    /* enable promise mode */
    SET_BIT(eqos->mac_regs->unused_004[1],
            0x1);

    /* Set TX flow control parameters */
    /* Set Pause Time */
    SET_BIT(eqos->mac_regs->q0_tx_flow_ctrl,
            0xffff << EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT);
    /* Assign priority for TX flow control */
    CLEAR_BIT(eqos->mac_regs->txq_prty_map0,
              EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK << EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT);
    /* Assign priority for RX flow control */
    CLEAR_BIT(eqos->mac_regs->rxq_ctrl2,
              EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK << EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT);
    /* Enable flow control */
    SET_BIT(eqos->mac_regs->q0_tx_flow_ctrl,
            EQOS_MAC_Q0_TX_FLOW_CTRL_TFE);
    SET_BIT(eqos->mac_regs->rx_flow_ctrl,
            EQOS_MAC_RX_FLOW_CTRL_RFE);

    MODIFY_REG(eqos->mac_regs->configuration,
               EQOS_MAC_CONFIGURATION_GPSLCE |
                   EQOS_MAC_CONFIGURATION_WD |
                   EQOS_MAC_CONFIGURATION_JD |
                   EQOS_MAC_CONFIGURATION_JE,
               EQOS_MAC_CONFIGURATION_CST |
                   EQOS_MAC_CONFIGURATION_ACS);

    eqos_write_hwaddr(eqos);

    /* Configure DMA */

    /* Enable OSP mode */
    SET_BIT(eqos->dma_regs->ch0_tx_control,
            EQOS_DMA_CH0_TX_CONTROL_OSP);

    /* RX buffer size. Must be a multiple of bus width */
    MODIFY_REG(eqos->dma_regs->ch0_rx_control,
               EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK << EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT,
               EQOS_MAX_PACKET_SIZE << EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT);

    SET_BIT(eqos->dma_regs->ch0_control,
            EQOS_DMA_CH0_CONTROL_PBLX8);

    /*
     * Burst length must be < 1/2 FIFO size.
     * FIFO size in tqs is encoded as (n / 256) - 1.
     * Each burst is n * 8 (PBLX8) * 16 (AXI width) == 128 bytes.
     * Half of n * 256 is n * 128, so pbl == tqs, modulo the -1.
     */
    pbl = tqs + 1;
    if (pbl > 32)
        pbl = 32;
    MODIFY_REG(eqos->dma_regs->ch0_tx_control,
               EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK << EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT,
               pbl << EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT);

    MODIFY_REG(eqos->dma_regs->ch0_rx_control,
               EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK << EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT,
               8 << EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT);

    /* DMA performance configuration */
    val = (2 << EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT) |
          EQOS_DMA_SYSBUS_MODE_EAME | EQOS_DMA_SYSBUS_MODE_BLEN16 |
          EQOS_DMA_SYSBUS_MODE_BLEN8 | EQOS_DMA_SYSBUS_MODE_BLEN4;
    writel(val, &eqos->dma_regs->sysbus_mode);

    /* Set up descriptors */

    memset(eqos->descs, 0, EQOS_DESCRIPTORS_SIZE);
    for (i = 0; i < EQOS_DESCRIPTORS_RX; i++)
    {
        struct eqos_desc *rx_desc = &(eqos->rx_descs[i]);
        rx_desc->des0 = (uint32_t)(ulong)(eqos->rx_dma_buf +
                                          (i * EQOS_MAX_PACKET_SIZE));
        rx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
        PRT_DMB();
        eqos->config->ops->eqos_flush_desc(rx_desc);
        eqos->config->ops->eqos_inval_buffer(eqos->rx_dma_buf +
                                                 (i * EQOS_MAX_PACKET_SIZE),
                                             EQOS_MAX_PACKET_SIZE);
    }

    writel(0, &eqos->dma_regs->ch0_txdesc_list_haddress);
    writel((ulong)eqos->tx_descs, &eqos->dma_regs->ch0_txdesc_list_address);
    writel(EQOS_DESCRIPTORS_TX - 1,
           &eqos->dma_regs->ch0_txdesc_ring_length);

    writel(0, &eqos->dma_regs->ch0_rxdesc_list_haddress);
    writel((ulong)eqos->rx_descs, &eqos->dma_regs->ch0_rxdesc_list_address);
    writel(EQOS_DESCRIPTORS_RX - 1,
           &eqos->dma_regs->ch0_rxdesc_ring_length);
    
    /* Enable interrupt */
    val = EQOS_DMA_CH0_INTR_ENA_NIE | EQOS_DMA_CH0_INTR_ENA_RIE | EQOS_DMA_CH0_INTR_ENA_TIE |
          EQOS_DMA_CH0_INTR_ENA_AIE | EQOS_DMA_CH0_INTR_ENA_CDEE | EQOS_DMA_CH0_INTR_ENA_FBEE;
    writel(val, &eqos->dma_regs->ch0_interrupt_enable);

    /* Enable everything */
    SET_BIT(eqos->dma_regs->ch0_tx_control,
            EQOS_DMA_CH0_TX_CONTROL_ST);
    SET_BIT(eqos->dma_regs->ch0_rx_control,
            EQOS_DMA_CH0_RX_CONTROL_SR);
    SET_BIT(eqos->mac_regs->configuration,
            EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

    /* TX tail pointer not written until we need to TX a packet */
    /*
     * Point RX tail pointer at last descriptor. Ideally, we'd point at the
     * first descriptor, implying all descriptors were available. However,
     * that's not distinguishable from none of the descriptors being
     * available.
     */
    last_rx_desc = (ulong) & (eqos->rx_descs[(EQOS_DESCRIPTORS_RX - 1)]);
    writel(last_rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);

    eqos->started = true;
}

int eqos_start(struct eqos_priv *eqos)
{
    int ret;

    ret = eqos_init(eqos);
    if (ret)
        return ret;

    eqos_enable(eqos);

    return 0;
}

void eqos_stop(struct eqos_priv *eqos)
{
    int i;

    HAL_DBG("%s\n", __func__);

    if (!eqos->started)
        return;
    eqos->started = false;
    eqos->reg_access_ok = false;

    /* Disable TX DMA */
    CLEAR_BIT(eqos->dma_regs->ch0_tx_control,
              EQOS_DMA_CH0_TX_CONTROL_ST);

    /* Wait for TX all packets to drain out of MTL */
    for (i = 0; i < 1000000; i++)
    {
        uint32_t val = readl(&eqos->mtl_regs->txq0_debug);
        uint32_t trcsts = (val >> EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT) &
                          EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK;
        uint32_t txqsts = val & EQOS_MTL_TXQ0_DEBUG_TXQSTS;
        if ((trcsts != 1) && (!txqsts))
            break;
    }

    /* Turn off MAC TX and RX */
    CLEAR_BIT(eqos->mac_regs->configuration,
              EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

    /* Wait for all RX packets to drain out of MTL */
    for (i = 0; i < 1000000; i++)
    {
        uint32_t val = readl(&eqos->mtl_regs->rxq0_debug);
        uint32_t prxq = (val >> EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT) &
                        EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK;
        uint32_t rxqsts = (val >> EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT) &
                          EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK;
        if ((!prxq) && (!rxqsts))
            break;
    }

    /* Turn off RX DMA */
    CLEAR_BIT(eqos->dma_regs->ch0_rx_control, EQOS_DMA_CH0_RX_CONTROL_SR);

    if (eqos->phy)
    {
        phy_shutdown(eqos->phy);
    }
    if (eqos->config->ops->eqos_stop_clks)
        eqos->config->ops->eqos_stop_clks(eqos);

    HAL_DBG("%s: OK\n", __func__);
}

int eqos_send(struct eqos_priv *eqos, void *packet, int length)
{
    struct eqos_desc *tx_desc;
    int i;

    HAL_DBG("%s(packet=%p, length=%d):\n", __func__, packet, length);

    memcpy(eqos->tx_dma_buf, packet, length);
    eqos->config->ops->eqos_flush_buffer(eqos->tx_dma_buf, length);

    tx_desc = &(eqos->tx_descs[eqos->tx_desc_idx]);
    eqos->tx_desc_idx++;
    eqos->tx_desc_idx %= EQOS_DESCRIPTORS_TX;

    tx_desc->des0 = (ulong)eqos->tx_dma_buf;
    tx_desc->des1 = 0;
    tx_desc->des2 = EQOS_DESC2_IOC | length;
    /*
     * Make sure that if HW sees the _OWN write below, it will see all the
     * writes to the rest of the descriptor too.
     */
    tx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_FD | EQOS_DESC3_LD | length;
    eqos->config->ops->eqos_flush_desc(tx_desc);

    PRT_DMB();
    writel((ulong)(&(eqos->tx_descs[eqos->tx_desc_idx])),
           &eqos->dma_regs->ch0_txdesc_tail_pointer);

    for (i = 0; i < 1000000; i++)
    {
        eqos->config->ops->eqos_inval_desc(tx_desc);
        if (!(readl(&tx_desc->des3) & EQOS_DESC3_OWN))
            return 0;
    }

    HAL_DBG("%s: TX timeout\n", __func__);

    return -ETIMEDOUT;
}

int eqos_recv(struct eqos_priv *eqos, int flags, uchar **packetp)
{
    struct eqos_desc *rx_desc;
    int length;

    HAL_DBG("%s(flags=%x):\n", __func__, flags);

    rx_desc = &(eqos->rx_descs[eqos->rx_desc_idx]);
    eqos->config->ops->eqos_inval_desc(rx_desc);
    if (rx_desc->des3 & EQOS_DESC3_OWN)
    {
        HAL_DBG("%s: RX packet not available\n", __func__);
        return -EAGAIN;
    }

    *packetp = eqos->rx_dma_buf +
               (eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
    length = rx_desc->des3 & 0x7fff;
    HAL_DBG("%s: *packetp=%p, length=%d\n", __func__, *packetp, length);

    eqos->config->ops->eqos_inval_buffer(*packetp, length);

    return length;
}

int eqos_free_pkt(struct eqos_priv *eqos, uchar *packet, int length)
{
    uchar *packet_expected;
    struct eqos_desc *rx_desc;

    HAL_DBG("%s(packet=%p, length=%d)\n", __func__, packet, length);

    packet_expected = eqos->rx_dma_buf +
                      (eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
    if (packet != packet_expected)
    {
        HAL_DBG("%s: Unexpected packet (expected %p)\n", __func__,
                packet_expected);
        return -EINVAL;
    }

    eqos->config->ops->eqos_inval_buffer(packet, length);
    rx_desc = &(eqos->rx_descs[eqos->rx_desc_idx]);

    rx_desc->des0 = 0;
    PRT_DMB();
    eqos->config->ops->eqos_flush_desc(rx_desc);
    eqos->config->ops->eqos_inval_buffer(packet, length);
    rx_desc->des0 = (uint32_t)(ulong)packet;
    rx_desc->des1 = 0;
    rx_desc->des2 = 0;
    /*
     * Make sure that if HW sees the _OWN write below, it will see all the
     * writes to the rest of the descriptor too.
     */
    PRT_DMB();
    rx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_BUF1V | EQOS_DESC3_IOC;
    eqos->config->ops->eqos_flush_desc(rx_desc);

    writel((ulong)rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);

    eqos->rx_desc_idx++;
    eqos->rx_desc_idx %= EQOS_DESCRIPTORS_RX;

    return 0;
}

static int eqos_probe_resources_core(struct eqos_priv *eqos)
{
    int ret;

    HAL_DBG("%s\n", __func__);

    if (!eqos->descs)
    {
        HAL_DBG("%s: eqos_alloc_descs() failed\n", __func__);
        ret = -ENOMEM;
        goto err;
    }
    eqos->tx_descs = (struct eqos_desc *)eqos->descs;
    eqos->rx_descs = (eqos->tx_descs + EQOS_DESCRIPTORS_TX);
    HAL_DBG("%s: tx_descs=%p, rx_descs=%p\n", __func__, eqos->tx_descs,
            eqos->rx_descs);

    eqos->tx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_MAX_PACKET_SIZE);
    if (!eqos->tx_dma_buf)
    {
        HAL_DBG("%s: memalign(tx_dma_buf) failed\n", __func__);
        ret = -ENOMEM;
        goto err;
    }
    HAL_DBG("%s: tx_dma_buf=%p\n", __func__, eqos->tx_dma_buf);

    eqos->rx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_RX_BUFFER_SIZE);
    if (!eqos->rx_dma_buf)
    {
        HAL_DBG("%s: memalign(rx_dma_buf) failed\n", __func__);
        ret = -ENOMEM;
        goto err_free_tx_dma_buf;
    }
    HAL_DBG("%s: rx_dma_buf=%p\n", __func__, eqos->rx_dma_buf);

    eqos->rx_pkt = malloc(EQOS_MAX_PACKET_SIZE);
    if (!eqos->rx_pkt)
    {
        HAL_DBG("%s: malloc(rx_pkt) failed\n", __func__);
        ret = -ENOMEM;
        goto err_free_rx_dma_buf;
    }
    HAL_DBG("%s: rx_pkt=%p\n", __func__, eqos->rx_pkt);

    eqos->config->ops->eqos_inval_buffer(eqos->rx_dma_buf,
                                         EQOS_MAX_PACKET_SIZE * EQOS_DESCRIPTORS_RX);

    HAL_DBG("%s: OK\n", __func__);
    return 0;

err_free_rx_dma_buf:
    free(eqos->rx_dma_buf);
err_free_tx_dma_buf:
    free(eqos->tx_dma_buf);
err:

    HAL_DBG("%s: returns %d\n", __func__, ret);
    return ret;
}

static int eqos_remove_resources_core(struct eqos_priv *eqos)
{
    HAL_DBG("%s\n", __func__);

    free(eqos->rx_pkt);
    free(eqos->rx_dma_buf);
    free(eqos->tx_dma_buf);

    HAL_DBG("%s: OK\n", __func__);
    return 0;
}

/* board-specific Ethernet Interface initializations. */
int board_interface_eth_init(struct eqos_priv *eqos,
                             phy_interface_t interface_type)
{
    return 0;
}

static int eqos_probe_resources_stm32(struct eqos_priv *eqos)
{
    return 0;
}

static phy_interface_t eqos_get_interface_stm32(struct eqos_priv *eqos)
{
    return eqos->interface;
}

static int eqos_remove_resources_stm32(struct eqos_priv *eqos)
{
    return 0;
}

int eqos_probe(struct eqos_priv *eqos)
{
    int ret;

    HAL_DBG("%s:\n", __func__);

    if (eqos->regs == 0)
    {
        HAL_DBG_ERR("dev_read_addr() failed\n");
        return -ENODEV;
    }

    eqos->mac_regs = (void *)(uintptr_t)(eqos->regs + EQOS_MAC_REGS_BASE);
    eqos->mtl_regs = (void *)(uintptr_t)(eqos->regs + EQOS_MTL_REGS_BASE);
    eqos->dma_regs = (void *)(uintptr_t)(eqos->regs + EQOS_DMA_REGS_BASE);

    ret = eqos_probe_resources_core(eqos);
    if (ret < 0)
    {
        HAL_DBG_ERR("eqos_probe_resources_core() failed: %d\n", ret);
        return ret;
    }

    ret = eqos->config->ops->eqos_probe_resources(eqos);
    if (ret < 0)
    {
        HAL_DBG_ERR("eqos_probe_resources() failed: %d\n", ret);
        goto err_remove_resources_core;
    }

    if (!eqos->mii)
    {
        eqos->mii = malloc(sizeof(struct mii_dev));
        if (!eqos->mii)
        {
            HAL_DBG_ERR("mdio_alloc() failed\n");
            ret = -ENOMEM;
            goto err_remove_resources_tegra;
        }
        memset(eqos->mii, 0, sizeof(struct mii_dev));
        eqos->mii->read = eqos_mdio_read;
        eqos->mii->write = eqos_mdio_write;
        eqos->mii->priv = eqos;
    }

    HAL_DBG("%s: OK\n", __func__);
    return 0;

err_remove_resources_tegra:
    eqos->config->ops->eqos_remove_resources(eqos);
err_remove_resources_core:
    eqos_remove_resources_core(eqos);

    HAL_DBG("%s: returns %d\n", __func__, ret);
    return ret;
}

// static int eqos_remove(struct eqos_priv *eqos)
// {
//     HAL_DBG("%s(dev=%p):\n", __func__);

//     free(eqos->mii);
//     eqos->config->ops->eqos_remove_resources(eqos);

//     eqos_probe_resources_core(eqos);

//     HAL_DBG("%s: OK\n", __func__);
//     return 0;
// }

uint32_t eqos_intr_status(struct eqos_priv *eqos)
{
	uint32_t intr_status = readl(&eqos->dma_regs->ch0_status);
    writel(intr_status & 0x3fffc7, &eqos->dma_regs->ch0_status);
    return intr_status;
}

struct eqos_ops eqos_rockchip_ops = {
    .eqos_inval_desc = eqos_inval_desc_generic,
    .eqos_flush_desc = eqos_flush_desc_generic,
    .eqos_inval_buffer = eqos_inval_buffer_generic,
    .eqos_flush_buffer = eqos_flush_buffer_generic,
    .eqos_probe_resources = eqos_probe_resources_stm32,
    .eqos_remove_resources = eqos_remove_resources_stm32,
    .eqos_stop_resets = eqos_stop_resets_stm32,
    .eqos_start_resets = eqos_start_resets_stm32,
    .eqos_calibrate_pads = eqos_calibrate_pads_stm32,
    .eqos_disable_calibration = eqos_disable_calibration_stm32,
    .eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_stm32,
    .eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_stm32,
    .eqos_get_interface = eqos_get_interface_stm32,
};
