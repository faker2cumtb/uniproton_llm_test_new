#ifndef _RK_GMAC_H_
#define _RK_GMAC_H_

#include "dwc_eth_qos.h"

struct rk_gmac_platdata
{
    bool clock_input;
    int tx_delay;
    int rx_delay;
    int bus_id;
};

struct rk_gmac_dev
{
    struct eqos_priv eqos;
    struct rk_gmac_platdata platdata;
    int phy_interface;
};

static inline int rk_gmac_write_hwaddr(struct rk_gmac_dev *dev)
{
    return eqos_write_hwaddr(&dev->eqos);
}

static inline int rk_gmac_free_pkt(struct rk_gmac_dev *dev, uchar *packet, int length)
{
    return eqos_free_pkt(&dev->eqos, packet, length);
}

static inline int rk_gmac_send(struct rk_gmac_dev *dev, void *packet, int length)
{
    return eqos_send(&dev->eqos, packet, length);
}

static inline int rk_gmac_recv(struct rk_gmac_dev *dev, int flags, uchar **packetp)
{
    return eqos_recv(&dev->eqos, flags, packetp);
}

static inline uint32_t rk_gmac_intr_status(struct rk_gmac_dev *dev)
{
    return eqos_intr_status(&dev->eqos);
}

int rk_gmac_probe(struct rk_gmac_dev *dev);
int rk_gmac_start(struct rk_gmac_dev *dev);
void rk_gmac_stop(struct rk_gmac_dev *dev);

#endif
