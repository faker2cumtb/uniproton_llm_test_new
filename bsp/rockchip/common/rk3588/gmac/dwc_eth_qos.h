/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020
 */

#ifndef _DWC_ETH_QOS_H
#define _DWC_ETH_QOS_H

#include <stdint.h>
#include "bitops.h"
#include "phy.h"
#include "hal_base.h"
#include "prt_buildef.h"

#define ARCH_DMA_MINALIGN	OS_CACHE_LINE_SIZE

#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_NOT_ENABLED		0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB		2
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV		1

#define EQOS_MAC_MDIO_ADDRESS_CR_100_150		1
#define EQOS_MAC_MDIO_ADDRESS_CR_20_35			2
#define EQOS_MAC_MDIO_ADDRESS_CR_250_300		5

/* Interrupt enable bits */
#define EQOS_DMA_CH0_INTR_ENA_NIE BIT(15)
#define EQOS_DMA_CH0_INTR_ENA_AIE BIT(14)
#define EQOS_DMA_CH0_INTR_ENA_CDEE BIT(13)
#define EQOS_DMA_CH0_INTR_ENA_FBEE BIT(12)
#define EQOS_DMA_CH0_INTR_ENA_ERIE BIT(11)
#define EQOS_DMA_CH0_INTR_ENA_ETIE BIT(10)
#define EQOS_DMA_CH0_INTR_ENA_RWTE BIT(9)
#define EQOS_DMA_CH0_INTR_ENA_RSE BIT(8)
#define EQOS_DMA_CH0_INTR_ENA_RBUE BIT(7)
#define EQOS_DMA_CH0_INTR_ENA_RIE BIT(6)
#define EQOS_DMA_CH0_INTR_ENA_TBUE BIT(2)
#define EQOS_DMA_CH0_INTR_ENA_TXSE BIT(1)
#define EQOS_DMA_CH0_INTR_ENA_TIE BIT(0)

struct eqos_priv {
	const struct eqos_config *config;
	uint32_t regs;
	struct eqos_mac_regs *mac_regs;
	struct eqos_mtl_regs *mtl_regs;
	struct eqos_dma_regs *dma_regs;

    struct mii_dev *mii;
	struct phy_device *phy;
	int phy_addr;
	uint32_t max_speed;

    unsigned char mac_addr[6];
    phy_interface_t interface;

	void *descs;
	struct eqos_desc *tx_descs;
	struct eqos_desc *rx_descs;
	int tx_desc_idx, rx_desc_idx;
	void *tx_dma_buf;
	void *rx_dma_buf;
	void *rx_pkt;
	bool started;
	bool reg_access_ok;
	bool mii_reseted;
};

struct eqos_ops {
	void (*eqos_inval_desc)(void *desc);
	void (*eqos_flush_desc)(void *desc);
	void (*eqos_inval_buffer)(void *buf, size_t size);
	void (*eqos_flush_buffer)(void *buf, size_t size);
	int (*eqos_probe_resources)(struct eqos_priv *eqos);
	int (*eqos_remove_resources)(struct eqos_priv *eqos);
	int (*eqos_stop_resets)(struct eqos_priv *eqos);
	int (*eqos_start_resets)(struct eqos_priv *eqos);
	void (*eqos_stop_clks)(struct eqos_priv *eqos);
	int (*eqos_start_clks)(struct eqos_priv *eqos);
	int (*eqos_calibrate_pads)(struct eqos_priv *eqos);
	int (*eqos_disable_calibration)(struct eqos_priv *eqos);
	int (*eqos_set_tx_clk_speed)(struct eqos_priv *eqos);
	ulong (*eqos_get_tick_clk_rate)(struct eqos_priv *eqos);
	phy_interface_t (*eqos_get_interface)(struct eqos_priv *eqos);
};

struct eqos_config {
	bool reg_access_always_ok;
	int mdio_wait;
	int swr_wait;
	int config_mac;
	int config_mac_mdio;
	struct eqos_ops *ops;
};

int eqos_init(struct eqos_priv *eqos);
void eqos_enable(struct eqos_priv *eqos);
int eqos_probe(struct eqos_priv *eqos);
int eqos_start(struct eqos_priv *eqos);
void eqos_stop(struct eqos_priv *eqos);
int eqos_send(struct eqos_priv *eqos, void *packet, int length);
int eqos_recv(struct eqos_priv *eqos, int flags, uchar **packetp);
int eqos_free_pkt(struct eqos_priv *eqos, uchar *packet, int length);
int eqos_write_hwaddr(struct eqos_priv *eqos);
uint32_t eqos_intr_status(struct eqos_priv *eqos);

extern struct eqos_ops eqos_rockchip_ops;

#endif
