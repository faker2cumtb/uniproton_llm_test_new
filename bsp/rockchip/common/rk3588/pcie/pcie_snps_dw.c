
#include "prt_sys.h"
#include "prt_tick.h"
#include "prt_config.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "cpu_config.h"
#include "hal_def.h"
#include "hal_base.h"
#include "grf_rk3588.h"
#include "pcie_rk_phy.h"
#include "pcie_snps_dw.h"
#include "print.h"

/* Access sizes for PCI reads and writes */
enum pci_size_t {
	PCI_SIZE_8,
	PCI_SIZE_16,
	PCI_SIZE_32,
};

static int rk_pcie_read(uint64_t addr, int size, uint32_t *val)
{
	if ((uint64_t)addr & (size - 1)) {
		*val = 0;
		return PCIBIOS_UNSUPPORTED;
	}
	*val = reg_read32(addr);

	return PCIBIOS_SUCCESSFUL;
}

static int rk_pcie_write(uint64_t addr, int size, uint32_t val)
{
	if ((uint64_t)addr & (size - 1))
		return PCIBIOS_UNSUPPORTED;

	reg_write32(val, addr);

	return PCIBIOS_SUCCESSFUL;
}

static uint32_t rk_pcie_read_apb(uint64_t base_apb,
								uint32_t reg, size_t size)
{
	int ret = 0;
	uint32_t val = 0;

	ret = rk_pcie_read(base_apb + reg, size, &val);
	if (ret)
		PRT_Printf("Read APB address failed");

	return val;
}

static void rk_pcie_write_apb(uint64_t base_apb,
				uint32_t reg, size_t size, uint32_t val)
{
	rk_pcie_write(base_apb + reg, size, val);
}	

static inline uint32_t rk_pcie_readl_apb(uint64_t base_apb, uint32_t reg)
{
	return rk_pcie_read_apb(base_apb, reg, 0x4);
}

static inline void rk_pcie_writel_apb(uint64_t base_apb, \
									uint32_t reg, uint32_t val)
{
	rk_pcie_write_apb(base_apb, reg, 0x4, val);
}

static inline void rk_pcie_dbi_write_enable(uint64_t base_dbi, bool en)
{
	uint32_t val;

	val = reg_read32(base_dbi + PCIE_MISC_CONTROL_1_OFF);
	if (en)
		val |= PCIE_DBI_RO_WR_EN;
	else
		val &= ~PCIE_DBI_RO_WR_EN;
	reg_write32(val, base_dbi + PCIE_MISC_CONTROL_1_OFF);
}

static void rk_pcie_setup_host(uint64_t base_dbi)
{
	uint32_t val;

	rk_pcie_dbi_write_enable(base_dbi, true);

	/* setup RC BARs */
	reg_write32(PCI_BASE_ADDRESS_MEM_TYPE_64,
	       base_dbi + PCI_BASE_ADDRESS_0);
	reg_write32(0x0, base_dbi + PCI_BASE_ADDRESS_1);

	/* setup interrupt pins */
	val = reg_read32(base_dbi + PCI_INTERRUPT_LINE);
	val &= 0xffff00ff;
	val |= 0x00000100;
	reg_write32(val, base_dbi + PCI_INTERRUPT_LINE);

	/* setup bus numbers */
	val = reg_read32(base_dbi + PCI_PRIMARY_BUS);
	val &= 0xff000000;
	val |= 0x00ff0100;
	reg_write32(val, base_dbi + PCI_PRIMARY_BUS);
	val = reg_read32(base_dbi + PCI_PRIMARY_BUS);

	/* setup command register */
	val = reg_read32(base_dbi + PCI_COMMAND);
	val &= 0xffff0000;
	val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
		PCI_COMMAND_MASTER | PCI_COMMAND_SERR;
	reg_write32(val, base_dbi + PCI_COMMAND);

	/* program correct class for RC */
	reg_write16(PCI_CLASS_BRIDGE_PCI, base_dbi + PCI_CLASS_DEVICE);

	/* Better disable write permission right after the update */
	val = reg_read32(base_dbi + PCIE_LINK_WIDTH_SPEED_CONTROL);
	val |= PORT_LOGIC_SPEED_CHANGE;
	reg_write32(val, base_dbi + PCIE_LINK_WIDTH_SPEED_CONTROL);
	rk_pcie_dbi_write_enable(base_dbi, false);
}

static void rk_pcie_configure(uint64_t base_dbi, uint32_t cap_speed)
{
	uint32_t val;

	rk_pcie_dbi_write_enable(base_dbi, true);

	val = reg_read32(base_dbi + PCIE_LINK_CAPABILITY);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	reg_write32(val, base_dbi + PCIE_LINK_CAPABILITY);

	val = reg_read32(base_dbi + PCIE_LINK_CTL_2);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	reg_write32(val, base_dbi + PCIE_LINK_CTL_2);

	rk_pcie_dbi_write_enable(base_dbi, false);
}

static inline void rk_pcie_link_status_clear(uint64_t base_apb)
{
	rk_pcie_writel_apb(base_apb, PCIE_CLIENT_GENERAL_DEBUG, 0x0);
}

static inline void rk_pcie_disable_ltssm(uint64_t base_apb)
{
	rk_pcie_writel_apb(base_apb, 0x0, 0xc0008);
}

static inline void rk_pcie_enable_ltssm(uint64_t base_apb)
{
	rk_pcie_writel_apb(base_apb, 0x0, 0xc000c);
}

static int is_link_up(struct rk_pcie3_cfg *pcie)
{
	uint32_t val;

	val = rk_pcie_readl_apb(pcie->apbBase, PCIE_CLIENT_LTSSM_STATUS);
	if ((val & (RDLH_LINKUP | SMLH_LINKUP)) == 0x30000 &&
	    (val & HAL_GENMASK(5, 0)) == 0x11)
	{
		PRT_Printf("STEP 1: PCI Link UP val = 0x%x\n", val);
		return 1;
	}

	return 0;
}

static int rk_pcie_link_up(struct rk_pcie3_cfg *pcie, uint32_t cap_speed)
{
	int retries;

	if (is_link_up(pcie)) {
		PRT_Printf("PCI Link already up before configuration!\n");
		return 1;
	}

	rk_pcie_configure(pcie->dbiBase, cap_speed);

	rk_pcie_disable_ltssm(pcie->apbBase);

	rk_pcie_link_status_clear(pcie->apbBase);

	/* Enable LTSSM */
	rk_pcie_enable_ltssm(pcie->apbBase);

	for (retries = 0; retries < 50; retries++) {
		if (is_link_up(pcie)) {
			/* Link maybe in Gen switch recovery but we need to wait more 1s */
			HAL_DelayUs(1000);
			return 0;
		}
		HAL_DelayUs(10000);
	}

	return 0;
}

void pcie_region_get(struct rk_pcie_cfg *cfg, struct pcie_dw_data *data)
{

	for (int i = 0 ; i < cfg->ranges_count ; ++i) {
		int val = (cfg->ranges[i].flags >> 24) & 0x03;
		switch (val) {
		case 0x01:
			data->regions[PCIE_REGION_IO].bus_start = cfg->ranges[i].pcie_bus_addr;
			data->regions[PCIE_REGION_IO].phys_start = cfg->ranges[i].host_map_addr;
			data->regions[PCIE_REGION_IO].size = cfg->ranges[i].map_length;
			/* Linux & U-Boot avoids allocating PCI resources from address 0 */
			if (data->regions[PCIE_REGION_IO].bus_start < 0x1000) {
				data->regions[PCIE_REGION_IO].allocation_offset = 0x1000;
			}
			else {
				data->regions[PCIE_REGION_IO].allocation_offset = 0;
			}
			break;
		case 0x02:
			data->regions[PCIE_REGION_MEM].bus_start = cfg->ranges[i].pcie_bus_addr;
			data->regions[PCIE_REGION_MEM].phys_start = cfg->ranges[i].host_map_addr;
			data->regions[PCIE_REGION_MEM].size = cfg->ranges[i].map_length;
			/* Linux & U-Boot avoids allocating PCI resources from address 0 */
			if (data->regions[PCIE_REGION_MEM].bus_start < 0x1000) {
				data->regions[PCIE_REGION_MEM].allocation_offset = 0x1000;
			}
			else {
				data->regions[PCIE_REGION_MEM].allocation_offset = 0;
			}
			break;
		case 0x03:
			data->regions[PCIE_REGION_MEM64].bus_start = cfg->ranges[i].pcie_bus_addr;
			data->regions[PCIE_REGION_MEM64].phys_start = cfg->ranges[i].host_map_addr;
			data->regions[PCIE_REGION_MEM64].size = cfg->ranges[i].map_length;
			/* Linux & U-Boot avoids allocating PCI resources from address 0 */
			if (data->regions[PCIE_REGION_MEM64].bus_start < 0x1000) {
				data->regions[PCIE_REGION_MEM64].allocation_offset = 0x1000;
			}
			else {
				data->regions[PCIE_REGION_MEM64].allocation_offset = 0;
			}
			break;
		}
	}
	data->cfgspc_phys = data->regions[PCIE_REGION_IO].phys_start - \
					data->regions[PCIE_REGION_IO].size;
	data->cfgspc_size = data->regions[PCIE_REGION_IO].size;

	#ifdef PCIE_DW_DEBUG
	PRT_Printf("pcie oi space[bus=0x%llx, phys=0x%llx, size=0x%llx]\
			\r\npcie mem space[bus=0x%llx, phys=0x%llx, size=0x%llx]\
			\r\npcie mem64 space[bus=0x%llx, phys=0x%llx, size=0x%llx]\n",\
			data->regions[PCIE_REGION_IO].bus_start,\
			data->regions[PCIE_REGION_IO].phys_start,\
			data->regions[PCIE_REGION_IO].size,\
			data->regions[PCIE_REGION_MEM].bus_start,\
			data->regions[PCIE_REGION_MEM].phys_start,\
			data->regions[PCIE_REGION_MEM].size,\
			data->regions[PCIE_REGION_MEM64].bus_start,\
			data->regions[PCIE_REGION_MEM64].phys_start,\
			data->regions[PCIE_REGION_MEM64].size);
	#endif
}

static void rk_pcie_writel_ob_unroll(struct rk_pcie_cfg *cfg, \
				uint32_t index, uint32_t reg, uint32_t val)
{
	uint32_t offset = PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(index);

	reg_write32(val, cfg->dbi + offset + reg);
}

static uint32_t rk_pcie_readl_ob_unroll(struct rk_pcie_cfg *cfg, \
			uint32_t index, uint32_t reg)
{
	uint32_t offset = PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(index);

	return reg_read32(cfg->dbi + offset + reg);
}

static void rk_pcie_prog_outbound_atu_unroll(struct rk_pcie_cfg *cfg, \
						int index,
					    int type, \
						uint64_t cpu_addr,
					    uint64_t pci_addr, \
						uint32_t size)
{
	uint32_t retries, val;

	#ifdef PCIE_DW_DEBUG
	PRT_Printf("ATU programmed with: index: %d, type: %d, \
			cpu addr: %8llx, pcie addr: %8llx, size: %8x\n",
		index, type, cpu_addr, pci_addr, size);
	#endif

	rk_pcie_writel_ob_unroll(cfg, index, PCIE_ATU_UNR_LOWER_BASE,
				 (uint32_t)(cpu_addr));
	rk_pcie_writel_ob_unroll(cfg, index, PCIE_ATU_UNR_UPPER_BASE,
				 (uint32_t)(cpu_addr>>32));
	rk_pcie_writel_ob_unroll(cfg, index, PCIE_ATU_UNR_LIMIT,
				 (uint32_t)(cpu_addr + size - 1));
	rk_pcie_writel_ob_unroll(cfg, index, PCIE_ATU_UNR_LOWER_TARGET,
				 (uint32_t)(pci_addr));
	rk_pcie_writel_ob_unroll(cfg, index, PCIE_ATU_UNR_UPPER_TARGET,
				 (uint32_t)(pci_addr>>32));
	rk_pcie_writel_ob_unroll(cfg, index, PCIE_ATU_UNR_REGION_CTRL1,
				 type);
	rk_pcie_writel_ob_unroll(cfg, index, PCIE_ATU_UNR_REGION_CTRL2,
				 PCIE_ATU_ENABLE);
	/*
	 * Make sure ATU enable takes effect before any subsequent config
	 * and I/O accesses.
	 */
	for (retries = 0; retries < LINK_WAIT_MAX_IATU_RETRIES; retries++) {
		val = rk_pcie_readl_ob_unroll(cfg, index,
					      PCIE_ATU_UNR_REGION_CTRL2);
		if (val & PCIE_ATU_ENABLE){
			#ifdef PCIE_DW_DEBUG
			PRT_Printf("outbound iATU is enabled %s %d index:%d\n", __func__, __LINE__, index);
			#endif
			return;
		}

		HAL_DelayUs(LINK_WAIT_IATU);
	}

	PRT_Printf("outbound iATU is not being enabled\n");
}

static int rk_pcie_addr_valid(pcie_id_t d, int first_busno)
{
	if ((PCI_BUS(d) == first_busno) && (PCI_DEV(d) > 0))
		return 0;
	if ((PCI_BUS(d) == first_busno + 1) && (PCI_DEV(d) > 0))
		return 0;

	return 1;
}

static uint64_t set_cfg_address(struct rk_pcie_cfg *cfg,
				struct pcie_dw_data *pcie_data,
				pcie_id_t d, uint32_t where)
{
	int bus = PCI_BUS(d) - cfg->bus_range_min;
	uint64_t va_address;
	uint32_t atu_type;

	/* Use dbi_base for own configuration read and write */
	if (!bus) {
		va_address = (uint64_t)cfg->dbi;
		goto out;
	}

	if (bus == 1)
		/*
		 * For local bus whose primary bus number is root bridge,
		 * change TLP Type field to 4.
		 */
		atu_type = PCIE_ATU_TYPE_CFG0;
	else
		/* Otherwise, change TLP Type field to 5. */
		atu_type = PCIE_ATU_TYPE_CFG1;

	/*
	 * Not accessing root port configuration space?
	 * Region #0 is used for Outbound CFG space access.
	 * Direction = Outbound
	 * Region Index = 0
	 */
	pcie_id_t pci_addr = PCI_MASK_BUS(d << 8U);
	pci_addr = PCI_ADD_BUS(bus, pci_addr);

	rk_pcie_prog_outbound_atu_unroll(cfg, 0,
					 atu_type, (uint64_t)pcie_data->cfgspc_phys,
					 pci_addr << 8U, pcie_data->cfgspc_size);

	va_address = (uint64_t)pcie_data->cfgspc_phys;

out:
	va_address += where;

	return va_address;
}

uint64_t pci_conv_32_to_size(uint64_t value, uint32_t offset, enum pci_size_t size)
{
	switch (size) {
	case PCI_SIZE_8:
		return (value >> ((offset & 3) * 8)) & 0xff;
	case PCI_SIZE_16:
		return (value >> ((offset & 2) * 8)) & 0xffff;
	default:
		return value;
	}
}

uint64_t pci_conv_size_to_32(uint64_t old, uint64_t value, uint32_t offset,
			  enum pci_size_t size)
{
	uint32_t off_mask;
	uint32_t val_mask, shift;
	uint64_t ldata, mask;

	switch (size) {
	case PCI_SIZE_8:
		off_mask = 3;
		val_mask = 0xff;
		break;
	case PCI_SIZE_16:
		off_mask = 2;
		val_mask = 0xffff;
		break;
	default:
		return value;
	}
	shift = (offset & off_mask) * 8;
	ldata = (value & val_mask) << shift;
	mask = val_mask << shift;
	value = (old & ~mask) | ldata;

	return value;
}

int pci_get_ff(enum pci_size_t size)
{
	switch (size) {
	case PCI_SIZE_8:
		return 0xff;
	case PCI_SIZE_16:
		return 0xffff;
	default:
		return 0xffffffff;
	}
}

static uint32_t rk_pcie_rd_conf(const struct pci_driver *pdrv, pcie_bdf_t bdf,
					  unsigned int reg)
{
	uint64_t va_address;
	uint64_t value;
	struct rk_pcie_cfg *cfg = (struct rk_pcie_cfg*)pdrv->config;
	struct pcie_dw_data *data = (struct pcie_dw_data*)pdrv->data;

	#ifdef PCIE_DW_DEBUG
	PRT_Printf("PCIE CFG read: bdf=%2x:%2x:%2x\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	#endif

	if (!rk_pcie_addr_valid(bdf, cfg->bus_range_min)) {
		//PRT_Printf("- out of range\n");
		value = pci_get_ff(PCI_SIZE_32);
		return value;
	}

	va_address = set_cfg_address(cfg, data, bdf, reg << 2);
	value = reg_read32(va_address);
	value = pci_conv_32_to_size(value, reg, PCI_SIZE_32);

	#ifdef PCIE_DW_DEBUG
	PRT_Printf("(addr,val)=(0x%04x, 0x%08lx)\n", reg << 2, value);
	#endif

	rk_pcie_prog_outbound_atu_unroll(cfg, PCIE_ATU_REGION_INDEX1,
					 PCIE_ATU_TYPE_IO, \
					 data->regions[PCIE_REGION_IO].phys_start, \
					 data->regions[PCIE_REGION_IO].bus_start, \
					 data->regions[PCIE_REGION_IO].size);

	return value;
}

static uint32_t rk_pcie_wr_conf(const struct pci_driver *pdrv, pcie_bdf_t bdf,
					  unsigned int reg, uint32_t data)
{
	struct rk_pcie_cfg *cfg = (struct rk_pcie_cfg*)pdrv->config;
	struct pcie_dw_data *dw_data = (struct pcie_dw_data*)pdrv->data;
	uint64_t va_address;
	uint64_t old;

	#ifdef PCIE_DW_DEBUG
	PRT_Printf("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d)\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	PRT_Printf("(addr,val)=(0x%04x, 0x%08lx)\n", reg << 2, data);
	#endif

	if (!rk_pcie_addr_valid(bdf, cfg->bus_range_min)) {
		return PCIBIOS_UNSUPPORTED;
	}

	va_address = set_cfg_address(cfg, dw_data, bdf, reg << 2);
	old = reg_read32(va_address);
	data = pci_conv_size_to_32(old, data, reg, PCI_SIZE_32);
	reg_write32(data, va_address);
	rk_pcie_prog_outbound_atu_unroll(cfg, PCIE_ATU_REGION_INDEX1,
					 PCIE_ATU_TYPE_IO, \
					 dw_data->regions[PCIE_REGION_IO].phys_start, \
					 dw_data->regions[PCIE_REGION_IO].bus_start, \
					 dw_data->regions[PCIE_REGION_IO].size);

	return PCIBIOS_UNSUPPORTED;
}

static bool pcie_dw_region_allocate_type(const struct pci_driver *pdrv, size_t bar_size, uint64_t *bar_bus_addr, enum pcie_region_type type)
{
	struct pcie_dw_data *data = (struct pcie_dw_data*)pdrv->data;
	uint64_t addr;

	addr = (((data->regions[type].bus_start + data->regions[type].allocation_offset) - 1) |
		((bar_size) - 1)) + 1;

	if (addr - data->regions[type].bus_start + bar_size > data->regions[type].size) {
		return false;
	}

	*bar_bus_addr = addr;
	data->regions[type].allocation_offset = addr - data->regions[type].bus_start + bar_size;

	return true;
}

static bool pcie_dw_region_allocate(const struct pci_driver *pdrv, bool mem64, size_t bar_size, uint64_t *bar_bus_addr)
{
	struct pcie_dw_data *data = (struct pcie_dw_data*)pdrv->data;
	enum pcie_region_type type;

	if (mem64 && !data->regions[PCIE_REGION_MEM64].size &&
	    !data->regions[PCIE_REGION_MEM].size) {
		PRT_Printf("No mem region defined for allocation");
		return false;
	}

	/*
	 * Allocate into mem64 region if available or is the only available
	 *
	 * TOFIX:
	 * - handle allocation from/to mem/mem64 when a region is full
	 */
	if (mem64 && data->regions[PCIE_REGION_MEM64].size) {
		type = PCIE_REGION_MEM64;
	} else {
		type = PCIE_REGION_MEM;
	}

	return pcie_dw_region_allocate_type(pdrv, bar_size, bar_bus_addr, type);
}

static bool pcie_dw_region_translate(const struct pci_driver *pdrv, bool mem64, uint64_t bar_bus_addr,
				       uint64_t *bar_addr)
{
	struct pcie_dw_data *data = (struct pcie_dw_data*)pdrv->data;
	enum pcie_region_type type;

	/* Means it hasn't been allocated */
	if (!bar_bus_addr) {
		return false;
	}

	if ((mem64 && data->regions[PCIE_REGION_MEM64].size)) {
		type = PCIE_REGION_MEM64;
	} else {
		type = PCIE_REGION_MEM;
	}

	*bar_addr = data->regions[type].phys_start + (bar_bus_addr - data->regions[type].bus_start);
	return true;
}


int pcie_dw_init(void *pcie_cfg, struct rk_pcie_cfg *pcie_ranges_cfg,
					struct pcie_dw_data *pcie_data)
{
	struct rk_pcie3_cfg *pcie = (struct rk_pcie3_cfg *)pcie_cfg;
	uint64_t cru_mp = pcie->cru;

	//phy 初始化和reset
	rk_p3phy_init(pcie);

	int bank = 131082 / 16;
	int offset = 131082 % 16;
	reg_write32((0x01 << offset) | ((0x01 << offset) << 16), cru_mp + 0xA00U + (bank * 4));

	HAL_DelayUs(1);

    reg_write32(((0x01 << offset) << 16), cru_mp + 0xA00U + (bank * 4));

	int shift_con32 = 525 % 16;
	reg_write32((0x01 << shift_con32) << 16, cru_mp + CRU_SOFTRST_CON32_OFFSET);
	
	int shift_con33 = 540 % 16;
	reg_write32((0x01 << shift_con33) << 16, cru_mp + CRU_SOFTRST_CON33_OFFSET);

	/* LTSSM EN ctrl mode */
	uint32_t val = rk_pcie_readl_apb(pcie->apbBase, PCIE_CLIENT_HOT_RESET_CTRL);
	val |= PCIE_LTSSM_ENABLE_ENHANCE | (PCIE_LTSSM_ENABLE_ENHANCE << 16);
	rk_pcie_writel_apb(pcie->apbBase, PCIE_CLIENT_HOT_RESET_CTRL, val);

	/* Set RC mode */
	rk_pcie_writel_apb(pcie->apbBase, 0x0, 0xf00040);
	rk_pcie_setup_host(pcie->dbiBase);
	rk_pcie_link_up(pcie, 3); //MAX lan

	pcie_region_get(pcie_ranges_cfg, pcie_data);

	pcie_ranges_cfg->apb = pcie->apbBase;
	pcie_ranges_cfg->dbi = pcie->dbiBase;
	rk_pcie_prog_outbound_atu_unroll(pcie_ranges_cfg, 
					PCIE_ATU_REGION_INDEX0, \
					PCIE_ATU_TYPE_MEM, \
					pcie_data->regions[PCIE_REGION_MEM].phys_start, \
					pcie_data->regions[PCIE_REGION_MEM].bus_start, \
					pcie_data->regions[PCIE_REGION_MEM].size);

	return 0;
}

struct pcie_ctrl_driver_api pcie_ctrl_api = {
	.conf_read = rk_pcie_rd_conf,
	.conf_write = rk_pcie_wr_conf,
	.region_allocate = pcie_dw_region_allocate,
	.region_translate = pcie_dw_region_translate,
};

void pcie_dw_api_conf(void *pci_drv)
{
	struct pci_driver *pdrv = (struct pci_driver*)pci_drv;
	pdrv->api = &pcie_ctrl_api;
}
