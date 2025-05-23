#include "prt_hwi.h"
#include "prt_clk.h"
#include "prt_sem.h"
#include "prt_task.h"
#include "prt_mem.h"
#include "hal_base.h"
#include "hwi_router.h"
#include "pcie_snps_dw.h"
#include "cpu_config.h"
#include "pcie_rk_phy.h"
#include "pcie.h"
#include "print.h"

#define PCIE_VENDOR_ID 0x144d
#define PCIE_DEVICE_ID 0xA804

#define PCIE3X4_APB_BASE        0xFE150000  /* PCIe apb base address */
#define PCIE3X4_DBI_BASE        0xA40000000 /* PCIe dbi base address */

struct rk_pcie3_cfg dw_pcie_cfg = {
	.phy_grf = 0xfd5b8000,
    .pipe_grf = 0xfd5b0000,
	.cru = 0xfd7c0000,
	.iobus = MMU_BUS_IOC_ADDR,
    .apbBase = PCIE3X4_APB_BASE,
    .dbiBase = PCIE3X4_DBI_BASE,
};

struct rk_pcie_cfg pcie_ranges_cfg = {
    .bus_range_min = 0,
    .ranges_count = 4,
    .ranges = {
        {0x00000800, 0xf0000000, 0xf0000000, 0x100000},
        {0x81000000, 0xf0100000, 0xf0100000, 0x100000},
        {0x82000000, 0xf0200000, 0xf0200000, 0xe00000},
        {0xc3000000, 0x900000000, 0x900000000, 0x40000000},
    },
};

static char g_dw_driver_name[] = "dw_pci_model";
static struct pci_device_id g_dw_pci_dev_tbl[] = {
    { PCI_DEVICE(PCIE_VENDOR_ID, PCIE_DEVICE_ID) },
    { 0, 0 },
};

int dw_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int32_t ret;

    PRT_Printf("Identify pcie device\n");
    PRT_Printf("dw_probe bdf:%02x:%02x.%01x\r\n", dev->bus_no,
        PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));

    ret = pci_enable_device(dev);
    if (ret != 0) {
        PRT_Printf("func:%s line:%d, ret:%x\r\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = pci_request_regions(dev, 0, &dev->resource[0]);
    if (ret != 0) {
        PRT_Printf("func:%s line:%d, ret:%x\r\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    pci_set_master(dev);

    uintptr_t bar0 = pci_resource_start(dev, 0);
    uint32_t bar0_size = pci_resource_len(dev, 0);
    uint32_t bar0_flags = pci_resource_flags(dev, 0);
    PRT_Printf("bar0:%llx size:0x%x flag:0x%x\n", bar0, bar0_size, bar0_flags);

    return 0;
}

void dw_remove(struct pci_dev *dev)
{
    if (dev)
    {
        PRT_MemFree(OS_MID_HARDDRV, dev);
    }
}

static struct pci_driver dw_driver = {
    .name       = g_dw_driver_name,
    .id_table   = g_dw_pci_dev_tbl,
    .probe      = dw_probe,
    .remove     = dw_remove,
};

void pcie_demo(void)
{
    struct pcie_dw_data pcie_data;
    struct pci_driver *pdrv = &dw_driver;

    PRT_Printf("Enter pcie demo\n");

    pcie_dw_init(&dw_pcie_cfg, &pcie_ranges_cfg, &pcie_data);
    pcie_dw_api_conf(pdrv);

    pdrv->config =  &pcie_ranges_cfg;
    pdrv->data = &pcie_data;
    pci_register_driver(&dw_driver);

    while (1)
    {
        PRT_Printf("RK 3588 PCIE test\n");
        PRT_ClkDelayMs(1000);
    }

}
