/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-10-17
 * Description: PCIE功能
 * Modify: 2024/11/17 lyp
 */

#include <prt_typedef.h>
#include <print.h>
#include "pcie_snps_dw.h"
#include "pcie.h"
#include "pcie_config.h"

/* arch agnostic PCIe API implementation */
static uint32_t pcie_conf_read(const struct pci_driver *pdrv, pcie_bdf_t bdf, unsigned int offset)
{
	if (!pdrv) {
		return 0xffffffff;
	}

	return pcie_ctrl_conf_read(pdrv, bdf, offset);
}

static uint32_t pcie_conf_write(const struct pci_driver *pdrv, pcie_bdf_t bdf, unsigned int offset, uint32_t data)
{
	if (!pdrv) {
		return 0xffffffff;
	}

	pcie_ctrl_conf_write(pdrv, bdf, offset, data);

    return 0;
}

int pcie_device_cfg_read(const struct pci_driver *pdrv, uint32_t bdf, uint32_t offset, uint32_t *val)
{
	if (!pdrv) {
		return 0xffffffff;
	}

	*val = pcie_conf_read(pdrv, bdf, offset >> 2U);

	return 0;
}

int pcie_device_cfg_write(const struct pci_driver *pdrv, uint32_t bdf, uint32_t offset, uint32_t val)
{
    if (!pdrv) {
		return 0xffffffff;
	}

	pcie_conf_write(pdrv, bdf, offset >> 2U, val);

    return 0;
}

void pcie_device_cfg_read_byte(struct pci_driver *pci_drv,
								uint32_t bdf, uint32_t offset, uint8_t *val)
{
    uint32_t full_val = pcie_conf_read(pci_drv, bdf, offset >> 2U);
	uint32_t bytes_offset = (offset % 4);

	full_val = (full_val >> (bytes_offset * 8));
	*val = full_val & 0xff;
}

void pcie_device_cfg_write_byte(struct pci_driver *pci_drv,
								uint32_t bdf, uint32_t offset, uint8_t val)
{
 	uint32_t full_val = pcie_conf_read(pci_drv, bdf, offset >> 2);
	uint32_t bytes_offset = (offset % 4U);

	full_val &=  ~(0xff << (bytes_offset * 8U));
	full_val = full_val | (val << (bytes_offset * 8U));
	pcie_conf_write(pci_drv, bdf, offset >> 2U, full_val);
}

void pcie_device_cfg_read_halfword(struct pci_driver *pci_drv,
								uint32_t bdf, uint32_t offset, uint16_t *val)
{
    uint32_t full_val = pcie_conf_read(pci_drv, bdf, offset >> 2);
	uint32_t bytes_offset = (offset % 4U);

	full_val = (full_val >> (bytes_offset * 8U));
	*val = full_val & 0xffff;
}

void pcie_device_cfg_write_halfword(struct pci_driver *pci_drv,
								uint32_t bdf, uint32_t offset, uint16_t val)
{
 	uint32_t full_val = pcie_conf_read(pci_drv, bdf, offset >> 2);
	uint32_t bytes_offset = (offset % 4U);

	full_val &=  ~(0xffff << (bytes_offset * 8U));
	full_val = full_val | (val << (bytes_offset * 8U));
	pcie_conf_write(pci_drv, bdf,  offset >> 2U, full_val);
}

int pcie_device_cfg_region_allocate(struct pci_driver *pci_drv, bool mem64, struct resource *res)
{
    uint64_t bar_bus_addr;
    pcie_bdf_t bdf = res->bdf;
    uint32_t bar = res->bar_index;
    size_t bar_size = res->size;

	if (pcie_ctrl_region_allocate(pci_drv, mem64, bar_size, &bar_bus_addr)) {
		uintptr_t bar_phys_addr;

		pcie_ctrl_region_translate(pci_drv, mem64, bar_bus_addr, &bar_phys_addr);
		PRT_Printf("[%02x:%02x.%x]  BAR[%d] size 0x%lx "
				"assigned [%s 0x%lx-0x%lx -> 0x%lx-0x%lx]\n",
				PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
				bar, bar_size,
				(mem64 ? "mem64" : "mem"),
				bar_bus_addr, bar_bus_addr + bar_size - 1,
				bar_phys_addr, bar_phys_addr + bar_size - 1);
		res->bar_phys_addr = bar_phys_addr;
        return 0;
	}
	else {
		PRT_Printf("[%02x:%02x.%x] BAR%d size 0x%lx Failed memory allocation.",
				PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
				bar, bar_size);
	}

    return -1;
}