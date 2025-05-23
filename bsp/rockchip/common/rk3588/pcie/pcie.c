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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prt_typedef.h"
#include "prt_module.h"
#include "prt_mem.h"
#include "securec.h"
#include "pcie_config.h"
#include "print.h"

#define IORESOURCE_IO   0x00000100
#define IORESOURCE_MEM  0x00000200

LIST_HEAD(g_pcie_device_list_head);
LIST_HEAD(g_pcie_driver_list_head);

int pci_match_dev(struct pci_driver *pci_drv, const struct pci_device_id *pci_dev_id_tbl,
                    uint32_t bdf, struct pci_device_id **pci_dev_id);
struct pci_dev *pci_dev_create_by_bdf(struct pci_driver *pci_drv, uint32_t bdf);
void pci_dev_add(struct pci_dev *pdev);

bool __attribute__((weak)) pci_bus_accessible(uint32_t bus_no)
{
    if (bus_no >= PCI_BUS_NUM_MAX) {
        return false;
    }
    return true;
}

/* 根据 pci_drv 中的 pci_device_id 匹配所有pci设备，若果能匹配到，则执行挂接probe函数 */
int pci_register_driver(struct pci_driver *pci_drv)
{
    int match = -1;
    int ret = OS_OK;
    uint32_t b, d, f;
    uint32_t bdf;
    const struct pci_device_id *pci_dev_id_tbl;
    struct pci_device_id *pci_dev_id;
    struct pci_dev *pci_device;

    if (pci_drv == NULL) {
        return ret;
    }

    pci_dev_id_tbl = pci_drv->id_table;
    for (b = 0; b < 2/*PCI_BUS_NUM_MAX*/; b++) {
        if (pci_bus_accessible(b) == false) {
            continue;
        }

        for (d = 0; d < PCI_DEIVCE_NUM_MAX; d++) {
            for (f = 0; f < PCI_FUNCTION_NUM_MAX; f++) {
                bdf = (uint32_t)PCI_BDF(b, d, f);
                ret = pci_match_dev(pci_drv, pci_dev_id_tbl, bdf, &pci_dev_id);
                if (match != ret) {
                    match = ret;
                }
                if (ret == FALSE) {
                    continue;
                }
                pci_device = pci_dev_create_by_bdf(pci_drv, bdf);
                if (pci_device == NULL) {
                    ret = OS_FAIL;
                    return ret;
                }

                pci_device->pdrv = pci_drv;
                pci_dev_add(pci_device);
                ret = pci_drv->probe(pci_device, pci_dev_id);
                PRT_Printf("bdf:0x%x probe ret 0x%x!\r\n", bdf, ret);
            }
        }
    }

    return ret;
}

int pci_match_dev(struct pci_driver *pci_drv, const struct pci_device_id *pci_dev_id_tbl, uint32_t bdf,
    struct pci_device_id **pci_dev_id)
{
    uint16_t vendor, device, subvendor, subdevice;
    uint8_t headertype;
    const struct pci_device_id *pdid;

    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_VENDOR_ID, &vendor);
    pcie_device_cfg_read_halfword(pci_drv,bdf, PCI_DEVICE_ID, &device);
    if (vendor == 0xffff && device  == 0xffff) {
        return FALSE; /* 该槽位没有设备 */
    }

    pcie_device_cfg_read_byte(pci_drv, bdf, PCI_HEADER_TYPE, &headertype);
    if ((headertype & PCI_HEADER_TYPE_MASK) != PCI_HEADER_TYPE_NORMAL) {
        return FALSE; /* 非终端设备， 桥设备或者cardbus */
    }

    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_SUBSYSTEM_VENDOR_ID, &subvendor);
    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_SUBSYSTEM_ID, &subdevice);

    for (pdid = pci_dev_id_tbl; (pdid->vendor | pdid->device) != 0; pdid++) {
        if ( (pdid->device == PCI_ANY_ID || pdid->device == device) &&
            (pdid->vendor == PCI_ANY_ID || pdid->vendor == vendor) &&
            (pdid->subvendor == PCI_ANY_ID || pdid->subvendor == subvendor) &&
            (pdid->subdevice == PCI_ANY_ID || pdid->subdevice == subdevice)
        ) {
            *pci_dev_id = (struct pci_device_id *)pdid; /* 记录该id,并返回匹配OK */
            return TRUE;
        }
    }
    return FALSE;
}

static inline unsigned long decode_bar(uint32_t bar)
{
    uint32_t mem_type;
    unsigned long flags;

    if ((bar & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO) {
        flags = bar & ~PCI_BASE_ADDRESS_IO_MASK;
        flags |= IORESOURCE_IO;
        return flags;
    }

    flags = bar & ~PCI_BASE_ADDRESS_MEM_MASK;
    flags |= IORESOURCE_MEM;
    if (flags & PCI_BASE_ADDRESS_MEM_PREFETCH)
        flags |= IORESOURCE_PREFETCH;

    mem_type = bar & PCI_BASE_ADDRESS_MEM_TYPE_MASK;
    switch (mem_type) {
    case PCI_BASE_ADDRESS_MEM_TYPE_32:
        break;
    case PCI_BASE_ADDRESS_MEM_TYPE_1M:
        /* 1M mem BAR treated as 32-bit BAR */
        break;
    case PCI_BASE_ADDRESS_MEM_TYPE_64:
        flags |= IORESOURCE_MEM_64;
        break;
    default:
        /* mem unknown type treated as 32-bit BAR */
        break;
    }
    return flags;
}

static uint64_t pci_size(uint64_t base, uint64_t maxbase, uint64_t mask)
{
    uint64_t size = mask & maxbase;    /* Find the significant bits */
    if (!size)
        return 0;

    /*
    * Get the lowest of them to find the decode size, and from that
    * the extent.
    */
    size = (size & ~(size-1)) - 1; 

    /*
    * base == maxbase can be valid only if the BAR has already been
    * programmed with all 1s.
    */
    if (base == maxbase && ((base | size) & mask) != mask)
        return 0;

    return size;
}

int pci_read_base(struct pci_dev *pdev, struct resource *res, uint32_t bar)
{
    uint32_t bdf;
    uint32_t l = 0, sz = 0, mask = ~0;
    uint64_t l64, sz64, mask64;
    bool found_mem64 = false;

    bdf = (pdev->bus_no << 8) | pdev->devfn;

    pcie_device_cfg_read(pdev->pdrv, bdf, PCI_REG_BAR(bar),&l);
    pcie_device_cfg_write(pdev->pdrv, bdf, PCI_REG_BAR(bar), mask);
    pcie_device_cfg_read(pdev->pdrv, bdf, PCI_REG_BAR(bar), &sz);
    pcie_device_cfg_write(pdev->pdrv, bdf, PCI_REG_BAR(bar), l);

    /*
    * All bits set in sz means the device isn't working properly.
    * If the BAR isn't implemented, all bits must be 0.  If it's a
    * memory BAR or a ROM, bit 0 must be clear; if it's an io BAR, bit
    * 1 must be clear.
    */
    if (sz == 0xffffffff)
        sz = 0;

    /*
    * I don't know how l can have all bits set.  Copied from old code.
    * Maybe it fixes a bug on some ancient platform.
    */
    if (l == 0xffffffff)
        l = 0;

    res->flags = decode_bar(l);
    res->flags |= IORESOURCE_SIZEALIGN;
    if (res->flags & IORESOURCE_IO) {
        l64 = l & PCI_BASE_ADDRESS_IO_MASK;
        sz64 = sz & PCI_BASE_ADDRESS_IO_MASK;
        mask64 = PCI_BASE_ADDRESS_IO_MASK & (uint32_t)IO_SPACE_LIMIT;
    } else {
        l64 = l & PCI_BASE_ADDRESS_MEM_MASK;
        sz64 = sz & PCI_BASE_ADDRESS_MEM_MASK;
        mask64 = (uint32_t)PCI_BASE_ADDRESS_MEM_MASK;
    }
   
    if (res->flags & IORESOURCE_MEM_64) {
        found_mem64 = true;
        pcie_device_cfg_read(pdev->pdrv, bdf, PCI_REG_BAR(bar + 1), &l);
        pcie_device_cfg_write(pdev->pdrv, bdf, PCI_REG_BAR(bar + 1), ~0);
        pcie_device_cfg_read(pdev->pdrv, bdf, PCI_REG_BAR(bar + 1), &sz);
        pcie_device_cfg_write(pdev->pdrv, bdf, PCI_REG_BAR(bar + 1), l);

        l64 |= ((uint64_t)l << 32);
        sz64 |= ((uint64_t)sz << 32);
        mask64 |= ((uint64_t)~0 << 32);
    }

    sz64 = pci_size(l64, sz64, mask64);
    if (!sz64) {
        res->flags = 0;
        PCIE_DBG_PRINTF("bar 0x%x: invalid BAR (can't size)\r\n", bar);
    }

    res->start = l64;
    res->end = l64 + sz64;
    res->bdf = bdf;
    res->size = sz64 + 1;
    res->mem64 = found_mem64;
    res->bar_index = bar;

    return (res->flags & IORESOURCE_MEM_64) ? 1 : 0;
}

/* Malloc and init pci_dev  */
struct pci_dev *pci_dev_create_by_bdf(struct pci_driver *pci_drv, uint32_t bdf)
{
    int bar;
    struct pci_dev *pdev;
    uint16_t class_revision, orig_cmd;

    pdev = (struct pci_dev*)PRT_MemAlloc(OS_MID_HARDDRV, OS_MEM_DEFAULT_FSC_PT,
        sizeof(struct pci_dev));
    if (pdev == NULL) {
        PCIE_DBG_PRINTF("pci dev create malloc fail\r\n");
        return NULL;
    }

    (void)memset_s(pdev, sizeof(struct pci_dev), 0, sizeof(struct pci_dev));
    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_VENDOR_ID, &(pdev->vendor));
    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_DEVICE_ID, &(pdev->device));
    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_SUBSYSTEM_VENDOR_ID, &(pdev->subsystem_vendor));
    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_SUBSYSTEM_ID, &(pdev->subsystem_device));
    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_CLASS_REVISION, &class_revision);
    pdev->class = class_revision & 0xff;
    pdev->revision = class_revision >> 8;

    pdev->bdf = bdf;
    pdev->bus_no = PCI_BUS(bdf);
    pdev->devfn = PCI_DEVFN_FROM_BDF(bdf);

    /* 这里需要配置 PCI_COMMAND 关闭 io 和 mem 空间访问，因为需要读写bar */
    pcie_device_cfg_read_halfword(pci_drv, bdf, PCI_COMMAND, &orig_cmd);
    if (orig_cmd & PCI_COMMAND_DECODE_ENABLE) {
        pcie_device_cfg_write_halfword(pci_drv, bdf, PCI_COMMAND,
            orig_cmd & ~PCI_COMMAND_DECODE_ENABLE);
    }

    pdev->pdrv = pci_drv;
    for (bar = 0; bar < DEVICE_COUNT_RESOURCE; bar++) {
        /* 如果地址为64bit, 会占用2个bar寄存器, 此时返回1, 否则返回0 */
        bar += pci_read_base(pdev, &(pdev->resource[bar]), bar);
    }
 
    if (orig_cmd & PCI_COMMAND_DECODE_ENABLE)
        pcie_device_cfg_write_halfword(pci_drv, bdf, PCI_COMMAND, orig_cmd);
    else /* 这里强制开启 io & mem */
        pcie_device_cfg_write_halfword(pci_drv, bdf, PCI_COMMAND, orig_cmd | PCI_COMMAND_DECODE_ENABLE);

    return pdev;
}

void pci_dev_add(struct pci_dev *pdev)
{
    list_add_tail(&(pdev->links), &g_pcie_device_list_head);
    if (pdev->pdrv->links.prev == NULL) { /* 避免重复push同一driver */
        list_add_tail(&(pdev->pdrv->links), &g_pcie_driver_list_head);
    }
}

int pci_irq_parse(char *buff, int *irq, int irq_num)
{
    char format[32];
    unsigned int cnt;
    char *ptr;

    for (cnt = 0; cnt < irq_num; cnt++) {
        sprintf(format, "msi_%u_", cnt);
        ptr = strstr(buff, format);
        if (ptr == NULL) {
            break;
        }

        ptr += strlen(format);
        irq[cnt] = atoi(ptr);
        PCIE_DBG_PRINTF("func:%s line:%d irq[%d]:%d\r\n",
            __FUNCTION__, __LINE__, cnt, irq[cnt]);
    }

    return cnt;
}

int pci_irq_vector(struct pci_dev *dev, int i)
{
    if (dev && dev->msi_enabled && i < PCI_IRQ_MAX_NUM) {
        return dev->irq[i];
    }
    return 0;
}

void pci_free_irq_vectors(struct pci_dev *dev)
{
    return;
}

int pci_enable_device(struct pci_dev *dev)
{
    uint16_t val;

    if (dev == NULL) {
        return -1;
    }

    pcie_device_cfg_read_halfword(dev->pdrv, dev->bdf, PCI_COMMAND, &val);
    val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
    pcie_device_cfg_write_halfword(dev->pdrv, dev->bdf, PCI_COMMAND, val);

    return 0;
}

int pci_disable_device(struct pci_dev *dev)
{
    uint16_t val;

    if (dev == NULL) {
        return -1;
    }

    pcie_device_cfg_read_halfword(dev->pdrv, dev->bdf, PCI_COMMAND, &val);
    val &= ~PCI_COMMAND_MASTER;
    pcie_device_cfg_write_halfword(dev->pdrv, dev->bdf, PCI_COMMAND, val);

    return 0;
}

void pci_set_master(struct pci_dev *dev)
{
    uint16_t val;

    if (dev == NULL) {
        return;
    }

    pcie_device_cfg_read_halfword(dev->pdrv, dev->bdf, PCI_COMMAND, &val);
    val |= PCI_COMMAND_MASTER;
    pcie_device_cfg_write_halfword(dev->pdrv, dev->bdf, PCI_COMMAND, val);

    return;
}

int pci_request_regions(struct pci_dev *pdev, bool mem64, struct resource *res)
{
    pcie_bdf_t bdf = res->bdf;
    uint32_t bar = res->bar_index;

    if (!pcie_device_cfg_region_allocate(pdev->pdrv, mem64, res)) {
		uint32_t reg = 0x10 + bar * 4U;

		pcie_device_cfg_write(pdev->pdrv, bdf, reg, res->bar_phys_addr & 0xFFFFFFFF);
		if (mem64) {
			pcie_device_cfg_write(pdev->pdrv, bdf, reg + 4U, res->bar_phys_addr >> 32U);
		}

        return 0;
	}

    return -1;
}

void pci_release_regions(struct pci_dev *pdev)
{
    return;
}