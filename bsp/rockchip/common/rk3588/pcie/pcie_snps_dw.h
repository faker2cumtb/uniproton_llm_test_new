
#ifndef __PCIE_SNPS_DW_H__
#define __PCIE_SNPS_DW_H__

#include "pcie.h"

#define BIT(nr) (1ULL << (nr))

/* Parameters for the waiting for iATU enabled routine */
#define PCIE_CLIENT_GENERAL_DEBUG	    0x104
#define PCIE_CLIENT_HOT_RESET_CTRL	    0x180
#define PCIE_LTSSM_ENABLE_ENHANCE	    BIT(4)
#define PCIE_CLIENT_LTSSM_STATUS	    0x300
#define SMLH_LINKUP			            BIT(16)
#define RDLH_LINKUP			            BIT(17)
#define PCIE_CLIENT_DBG_FIFO_MODE_CON	0x310
#define PCIE_CLIENT_DBG_FIFO_PTN_HIT_D0 0x320
#define PCIE_CLIENT_DBG_FIFO_PTN_HIT_D1 0x324
#define PCIE_CLIENT_DBG_FIFO_TRN_HIT_D0 0x328
#define PCIE_CLIENT_DBG_FIFO_TRN_HIT_D1 0x32c
#define PCIE_CLIENT_DBG_FIFO_STATUS	    0x350
#define PCIE_CLIENT_DBG_TRANSITION_DATA	0xffff0000
#define PCIE_CLIENT_DBF_EN		        0xffff0003
#define RK_PCIE_DBG			            0

/* PCI DBICS registers */
#define PCIE_LINK_STATUS_REG		    0x80
#define PCIE_LINK_STATUS_SPEED_OFF	    16
#define PCIE_LINK_STATUS_SPEED_MASK	    (0xf << PCIE_LINK_STATUS_SPEED_OFF)
#define PCIE_LINK_STATUS_WIDTH_OFF	    20
#define PCIE_LINK_STATUS_WIDTH_MASK	    (0xf << PCIE_LINK_STATUS_WIDTH_OFF)

#define PCIE_LINK_CAPABILITY		    0x7c
#define PCIE_LINK_CTL_2			        0xa0
#define TARGET_LINK_SPEED_MASK		    0xf
#define LINK_SPEED_GEN_1		        0x1
#define LINK_SPEED_GEN_2		        0x2
#define LINK_SPEED_GEN_3		        0x3

#define PCIE_MISC_CONTROL_1_OFF		    0x8bc
#define PCIE_DBI_RO_WR_EN		        BIT(0)

#define PCIE_LINK_WIDTH_SPEED_CONTROL	0x80c
#define PORT_LOGIC_SPEED_CHANGE		BIT(17)

/*
 * iATU Unroll-specific register definitions
 * From 4.80 core version the address translation will be made by unroll.
 * The registers are offset from atu_base
 */
#define PCIE_ATU_UNR_REGION_CTRL1	0x00
#define PCIE_ATU_UNR_REGION_CTRL2	0x04
#define PCIE_ATU_UNR_LOWER_BASE		0x08
#define PCIE_ATU_UNR_UPPER_BASE		0x0c
#define PCIE_ATU_UNR_LIMIT		    0x10
#define PCIE_ATU_UNR_LOWER_TARGET	0x14
#define PCIE_ATU_UNR_UPPER_TARGET	0x18

#define PCIE_ATU_REGION_INDEX1		(0x1 << 0)
#define PCIE_ATU_REGION_INDEX0		(0x0 << 0)
#define PCIE_ATU_TYPE_MEM		    (0x0 << 0)
#define PCIE_ATU_TYPE_IO		    (0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		    (0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		    (0x5 << 0)
#define PCIE_ATU_ENABLE			    (0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE	(0x1 << 30)
#define PCIE_ATU_BUS(x)			    (((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			    (((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		    (((x) & 0x7) << 16)

/* Register address builder */
#define PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(region)        \
	((0x3 << 20) | ((region) << 9))
#define PCIE_GET_ATU_INB_UNR_REG_OFFSET(region) \
	((0x3 << 20) | ((region) << 9) | (0x1 << 8))

/* Parameters for the waiting for iATU enabled routine */
#define LINK_WAIT_MAX_IATU_RETRIES	5
#define LINK_WAIT_IATU				10000

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define PCI_BASE_ADDRESS_0	                0x10	/* 32 bits */
#define PCI_BASE_ADDRESS_1	                0x14	/* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2	                0x18	/* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3	                0x1c	/* 32 bits */
#define PCI_BASE_ADDRESS_4	                0x20	/* 32 bits */
#define PCI_BASE_ADDRESS_5	                0x24	/* 32 bits */
#define PCI_BASE_ADDRESS_SPACE              0x01	/* 0 = memory, 1 = I/O */
#define PCI_BASE_ADDRESS_SPACE_IO           0x01
#define PCI_BASE_ADDRESS_SPACE_MEMORY       0x00
#define PCI_BASE_ADDRESS_MEM_TYPE_MASK      0x06
#define PCI_BASE_ADDRESS_MEM_TYPE_32	    0x00	/* 32 bit address */
#define PCI_BASE_ADDRESS_MEM_TYPE_1M	    0x02	/* Below 1M [obsolete] */
#define PCI_BASE_ADDRESS_MEM_TYPE_64	    0x04	/* 64 bit address */
#define PCI_BASE_ADDRESS_MEM_PREFETCH	    0x08	/* prefetchable? */

/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE	                0x3c	/* 8 bits */
#define PCI_INTERRUPT_PIN	                0x3d	/* 8 bits */
#define PCI_MIN_GNT		                    0x3e	/* 8 bits */
#define PCI_MAX_LAT		                    0x3f	/* 8 bits */

#define PCI_INTERRUPT_LINE_DISABLE	        0xff

/* Header type 1 (PCI-to-PCI bridges) */
#define PCI_PRIMARY_BUS		                0x18	/* Primary bus number */
#define PCI_SECONDARY_BUS	                0x19	/* Secondary bus number */
#define PCI_SUBORDINATE_BUS	                0x1a	/* Highest bus number behind the bridge */
#define PCI_SEC_LATENCY_TIMER	            0x1b	/* Latency timer for secondary interface */
#define PCI_IO_BASE		                    0x1c	/* I/O range behind the bridge */
#define PCI_IO_LIMIT		                0x1d
#define PCI_IO_RANGE_TYPE_MASK              0x0f	/* I/O bridging type */
#define PCI_IO_RANGE_TYPE_16	            0x00
#define PCI_IO_RANGE_TYPE_32	            0x01
#define PCI_IO_RANGE_MASK	                ~0x0f
#define PCI_SEC_STATUS		                0x1e	/* Secondary status register, only bit 14 used */
#define PCI_MEMORY_BASE		                0x20	/* Memory range behind */
#define PCI_MEMORY_LIMIT	                0x22
#define PCI_MEMORY_RANGE_TYPE_MASK          0x0f
#define PCI_MEMORY_RANGE_MASK	            ~0x0f
#define PCI_PREF_MEMORY_BASE	            0x24	/* Prefetchable memory range behind */
#define PCI_PREF_MEMORY_LIMIT	            0x26
#define PCI_PREF_RANGE_TYPE_MASK            0x0f
#define PCI_PREF_RANGE_TYPE_32              0x00
#define PCI_PREF_RANGE_TYPE_64              0x01
#define PCI_PREF_RANGE_MASK	                ~0x0f
#define PCI_PREF_BASE_UPPER32	            0x28	/* Upper half of prefetchable memory range */
#define PCI_PREF_LIMIT_UPPER32	            0x2c
#define PCI_IO_BASE_UPPER16	                0x30	/* Upper half of I/O addresses */
#define PCI_IO_LIMIT_UPPER16	            0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#define PCI_ROM_ADDRESS1	                0x38	/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_BRIDGE_CONTROL	                0x3e
#define PCI_BRIDGE_CTL_PARITY	            0x01	/* Enable parity detection on secondary interface */
#define PCI_BRIDGE_CTL_SERR	                0x02	/* The same for SERR forwarding */
#define PCI_BRIDGE_CTL_NO_ISA	            0x04	/* Disable bridging of ISA ports */
#define PCI_BRIDGE_CTL_VGA	                0x08	/* Forward VGA addresses */
#define PCI_BRIDGE_CTL_MASTER_ABORT         0x20  /* Report master aborts */
#define PCI_BRIDGE_CTL_BUS_RESET            0x40	/* Secondary bus reset */
#define PCI_BRIDGE_CTL_FAST_BACK            0x80	/* Fast Back2Back enabled on secondary interface */

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#define PCI_COMMAND		                    0x04	/* 16 bits */
#define PCI_COMMAND_IO		                0x1	/* Enable response in I/O space */
#define PCI_COMMAND_MEMORY	                0x2	/* Enable response in Memory space */
#define PCI_COMMAND_MASTER	                0x4	/* Enable bus mastering */
#define PCI_COMMAND_SPECIAL	                0x8	/* Enable response to special cycles */
#define PCI_COMMAND_INVALIDATE              0x10	/* Use memory write and invalidate */
#define PCI_COMMAND_VGA_PALETTE             0x20	/* Enable palette snooping */
#define PCI_COMMAND_PARITY	                0x40	/* Enable parity checking */
#define PCI_COMMAND_WAIT	                0x80	/* Enable address/data stepping */
#define PCI_COMMAND_SERR	                0x100	/* Enable SERR */
#define PCI_COMMAND_FAST_BACK	            0x200	/* Enable back-to-back writes */

#define PCI_STATUS		                    0x06	/* 16 bits */
#define PCI_STATUS_CAP_LIST	            	0x10	/* Support Capability List */
#define PCI_STATUS_66MHZ	                0x20	/* Support 66 Mhz PCI 2.1 bus */
#define PCI_STATUS_UDF		                0x40	/* Support User Definable Features [obsolete] */
#define PCI_STATUS_FAST_BACK	            0x80	/* Accept fast-back to back */
#define PCI_STATUS_PARITY	                0x100	/* Detected parity error */
#define PCI_STATUS_DEVSEL_MASK             	0x600	/* DEVSEL timing */
#define PCI_STATUS_DEVSEL_FAST             	0x000
#define PCI_STATUS_DEVSEL_MEDIUM           	0x200
#define PCI_STATUS_DEVSEL_SLOW             	0x400
#define PCI_STATUS_SIG_TARGET_ABORT        	0x800 /* Set on target abort */
#define PCI_STATUS_REC_TARGET_ABORT       	0x1000 /* Master ack of " */
#define PCI_STATUS_REC_MASTER_ABORT        	0x2000 /* Set on master abort */
#define PCI_STATUS_SIG_SYSTEM_ERROR        	0x4000 /* Set when we drive SERR */
#define PCI_STATUS_DETECTED_PARITY         	0x8000 /* Set on parity error */

#define PCI_CLASS_REVISION	                0x08	/* High 24 bits are class, low 8
					                                    revision */
#define PCI_REVISION_ID		                0x08	/* Revision ID */
#define PCI_CLASS_PROG		                0x09	/* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE	                0x0a	/* Device class */
#define PCI_CLASS_CODE		                0x0b	/* Device class code */
#define PCI_CLASS_CODE_TOO_OLD	            0x00
#define PCI_CLASS_CODE_STORAGE              0x01
#define PCI_CLASS_CODE_NETWORK              0x02
#define PCI_CLASS_CODE_DISPLAY	            0x03
#define PCI_CLASS_CODE_MULTIMEDIA           0x04
#define PCI_CLASS_CODE_MEMORY	            0x05
#define PCI_CLASS_CODE_BRIDGE	            0x06
#define PCI_CLASS_CODE_COMM	                0x07
#define PCI_CLASS_CODE_PERIPHERAL           0x08
#define PCI_CLASS_CODE_INPUT	            0x09
#define PCI_CLASS_CODE_DOCKING	            0x0A
#define PCI_CLASS_CODE_PROCESSOR            0x0B
#define PCI_CLASS_CODE_SERIAL	            0x0C
#define PCI_CLASS_CODE_WIRELESS             0x0D
#define PCI_CLASS_CODE_I2O	                0x0E
#define PCI_CLASS_CODE_SATELLITE            0x0F
#define PCI_CLASS_CODE_CRYPTO	            0x10
#define PCI_CLASS_CODE_DATA	0x11
/* Base Class 0x12 - 0xFE is reserved */
#define PCI_CLASS_CODE_OTHER	            0xFF


#define PCI_BASE_CLASS_BRIDGE		        0x06
#define PCI_CLASS_BRIDGE_HOST		        0x0600
#define PCI_CLASS_BRIDGE_ISA		        0x0601
#define PCI_CLASS_BRIDGE_EISA		        0x0602
#define PCI_CLASS_BRIDGE_MC		            0x0603
#define PCI_CLASS_BRIDGE_PCI		        0x0604
#define PCI_CLASS_BRIDGE_PCMCIA		        0x0605
#define PCI_CLASS_BRIDGE_NUBUS		        0x0606
#define PCI_CLASS_BRIDGE_CARDBUS	        0x0607
#define PCI_CLASS_BRIDGE_RACEWAY	        0x0608
#define PCI_CLASS_BRIDGE_OTHER		        0x0680

enum rk_pcie_device_mode {
	RK_PCIE_EP_TYPE,
	RK_PCIE_RC_TYPE,
};

/*
 * PCIe Controllers Regions
 *
 * TOFIX:
 * - handle prefetchable regions
 */
enum pcie_region_type {
	PCIE_REGION_IO = 0,
	PCIE_REGION_MEM,
	PCIE_REGION_MEM64,
	PCIE_REGION_MAX,
};

enum {
	PCIBIOS_SUCCESSFUL = 0x0000,
	PCIBIOS_UNSUPPORTED = -1,
	PCIBIOS_NODEV = -1,
};

struct pcie_dw_data {
	uintptr_t mp_dbi;	//memory mapped dbi base address
	uintptr_t mp_apb;	//memory mapped apb base address

	uintptr_t cfgspc_phys;	//cfg space base address
	uintptr_t cfgspc_base;	//cfg space base address
	size_t cfgspc_size;	//cfg space size

	size_t ranges_count;
	struct {
		uintptr_t phys_start;
		uintptr_t bus_start;
		size_t size;
		size_t allocation_offset;
	} regions[PCIE_REGION_MAX];
};

struct rk_pcie_cfg{
#ifdef CONFIG_OS_OPTION_PCIE_MSI
	const struct pci_dev *msi_parent;
#endif
	enum rk_pcie_device_mode mode;

	uintptr_t dbi;
	size_t dbi_size;

	uintptr_t apb;
	size_t apb_size;

	const struct pci_dev *phy;

	uint8_t bus_range_min;
	uint8_t bus_range_max;

	bool is_bifurcation;

	size_t ranges_count;
	struct {
		/* Flags as defined in the PCI Bus Binding to IEEE Std 1275-1994 */
		uint32_t flags;
		/* bus-centric offset from the start of the region */
		uintptr_t pcie_bus_addr;
		/* CPU-centric offset from the start of the region */
		uintptr_t host_map_addr;
		/* region size */
		size_t map_length;
	} ranges[4];
};

struct pcie_bus_state {
	/* Current scanned bus BDF, always valid */
	unsigned int bus_bdf;
	/* Current bridge endpoint BDF, either valid or PCIE_BDF_NONE */
	unsigned int bridge_bdf;
	/* Next BDF to scan on bus, either valid or PCIE_BDF_NONE when all EP scanned */
	unsigned int next_bdf;
};

/**
 * @brief Function called to read a 32-bit word from an endpoint's configuration space.
 *
 * Read a 32-bit word from an endpoint's configuration space with the PCI Express Controller
 * configuration space access method (I/O port, memory mapped or custom method)
 *
 * @param dev PCI Express Controller device pointer
 * @param bdf PCI(e) endpoint
 * @param reg the configuration word index (not address)
 * @return the word read (0xFFFFFFFFU if nonexistent endpoint or word)
 */
typedef uint32_t (*pcie_ctrl_conf_read_t)(const struct pci_driver *pdrv, pcie_bdf_t bdf,
					  unsigned int reg);

/**
 * @brief Function called to write a 32-bit word to an endpoint's configuration space.
 *
 * Write a 32-bit word to an endpoint's configuration space with the PCI Express Controller
 * configuration space access method (I/O port, memory mapped or custom method)
 *
 * @param dev PCI Express Controller device pointer
 * @param bdf PCI(e) endpoint
 * @param reg the configuration word index (not address)
 * @param data the value to write
 */
typedef uint32_t (*pcie_ctrl_conf_write_t)(const struct pci_driver *pdrv, pcie_bdf_t bdf,
					  unsigned int reg, uint32_t data);

/**
 * @brief Function called to allocate a memory region subset for an endpoint Base Address Register.
 *
 * When enumerating PCIe Endpoints, Type0 endpoints can require up to 6 memory zones
 * via the Base Address Registers from I/O or Memory types.
 *
 * This call allocates such zone in the PCI Express Controller memory regions if
 * such region is available and space is still available.
 *
 * @param dev PCI Express Controller device pointer
 * @param mem64 True if the BAR is of 64bit memory type
 * @param bar_size Size in bytes of the Base Address Register as returned by HW
 * @param bar_bus_addr bus-centric address allocated to be written in the BAR register
 * @return True if allocation was possible, False if allocation failed
 */
typedef bool (*pcie_ctrl_region_allocate_t)(const struct pci_driver *pdrv, bool mem64, size_t bar_size,
					    uintptr_t *bar_bus_addr);

/**
 * @brief Function called to translate an endpoint Base Address Register bus-centric address
 * into Physical address.
 *
 * When enumerating PCIe Endpoints, Type0 endpoints can require up to 6 memory zones
 * via the Base Address Registers from I/O or Memory types.
 *
 * The bus-centric address set in this BAR register is not necessarily accessible from the CPU,
 * thus must be translated by using the PCI Express Controller memory regions translation
 * ranges to permit mapping from the CPU.
 *
 * @param dev PCI Express Controller device pointer
 * @param bdf PCI(e) endpoint
 * @param mem True if the BAR is of memory type
 * @param mem64 True if the BAR is of 64bit memory type
 * @param bar_bus_addr bus-centric address written in the BAR register
 * @param bar_addr CPU-centric address translated from the bus-centric address
 * @return True if translation was possible, False if translation failed
 */
typedef bool (*pcie_ctrl_region_translate_t)(const struct pci_driver *pdrv, bool mem64, uintptr_t bar_bus_addr,
					     uintptr_t *bar_addr);

/** @brief Structure providing callbacks to be implemented for devices
 * that supports the PCI Express Controller API
 */
struct pcie_ctrl_driver_api {
	pcie_ctrl_conf_read_t conf_read;
	pcie_ctrl_conf_write_t conf_write;
	pcie_ctrl_region_allocate_t region_allocate;
	pcie_ctrl_region_translate_t region_translate;
#ifdef CONFIG_OS_OPTION_PCIE_MSI
	pcie_ctrl_msi_device_setup_t msi_device_setup;
#endif
};

/**
 * @brief Read a 32-bit word from a Memory-Mapped endpoint's configuration space.
 *
 * Read a 32-bit word from an endpoint's configuration space from a Memory-Mapped
 * configuration space access method, known as PCI Control Access Method (CAM) or
 * PCIe Extended Control Access Method (ECAM).
 *
 * @param cfg_addr Logical address of Memory-Mapped configuration space
 * @param bdf PCI(e) endpoint
 * @param reg the configuration word index (not address)
 * @return the word read (0xFFFFFFFFU if nonexistent endpoint or word)
 */
uint32_t pcie_generic_ctrl_conf_read(uint64_t cfg_addr, pcie_bdf_t bdf, unsigned int reg);

/**
 * @brief Write a 32-bit word to a Memory-Mapped endpoint's configuration space.
 *
 * Write a 32-bit word to an endpoint's configuration space from a Memory-Mapped
 * configuration space access method, known as PCI Control Access Method (CAM) or
 * PCIe Extended Control Access Method (ECAM).
 *
 * @param cfg_addr Logical address of Memory-Mapped configuration space
 * @param bdf PCI(e) endpoint
 * @param reg the configuration word index (not address)
 * @param data the value to write
 */
void pcie_generic_ctrl_conf_write(uint64_t cfg_addr, pcie_bdf_t bdf,
				  unsigned int reg, uint32_t data);

/**
 * @brief Read a 32-bit word from an endpoint's configuration space.
 *
 * Read a 32-bit word from an endpoint's configuration space with the PCI Express Controller
 * configuration space access method (I/O port, memory mapped or custom method)
 *
 * @param dev PCI Express Controller device pointer
 * @param bdf PCI(e) endpoint
 * @param reg the configuration word index (not address)
 * @return the word read (0xFFFFFFFFU if nonexistent endpoint or word)
 */
static inline uint32_t pcie_ctrl_conf_read(const struct pci_driver *pdrv, pcie_bdf_t bdf,
					   unsigned int reg)
{
	const struct pcie_ctrl_driver_api *api =
		(const struct pcie_ctrl_driver_api *)pdrv->api;

	return api->conf_read(pdrv, bdf, reg);
}

/**
 * @brief Write a 32-bit word to an endpoint's configuration space.
 *
 * Write a 32-bit word to an endpoint's configuration space with the PCI Express Controller
 * configuration space access method (I/O port, memory mapped or custom method)
 *
 * @param dev PCI Express Controller device pointer
 * @param bdf PCI(e) endpoint
 * @param reg the configuration word index (not address)
 * @param data the value to write
 */
static inline void pcie_ctrl_conf_write(const struct pci_driver *pdrv, pcie_bdf_t bdf,
					unsigned int reg, uint32_t data)
{
	const struct pcie_ctrl_driver_api *api =
		(const struct pcie_ctrl_driver_api *)pdrv->api;
	api->conf_write(pdrv, bdf, reg, data);
}

/**
 * @brief Allocate a memory region subset for an endpoint Base Address Register.
 *
 * When enumerating PCIe Endpoints, Type0 endpoints can require up to 6 memory zones
 * via the Base Address Registers from I/O or Memory types.
 *
 * This call allocates such zone in the PCI Express Controller memory regions if
 * such region is available and space is still available.
 *
 * @param dev PCI Express Controller device pointer
 * @param mem64 True if the BAR is of 64bit memory type
 * @param bar_size Size in bytes of the Base Address Register as returned by HW
 * @param bar_bus_addr bus-centric address allocated to be written in the BAR register
 * @return True if allocation was possible, False if allocation failed
 */
static inline bool pcie_ctrl_region_allocate(const struct pci_driver *pdrv, bool mem64, size_t bar_size,
					     uintptr_t *bar_bus_addr)
{
	const struct pcie_ctrl_driver_api *api =
		(const struct pcie_ctrl_driver_api *)pdrv->api;

	return api->region_allocate(pdrv, mem64, bar_size, bar_bus_addr);
}

/**
 * @brief Translate an endpoint Base Address Register bus-centric address into Physical address.
 *
 * When enumerating PCIe Endpoints, Type0 endpoints can require up to 6 memory zones
 * via the Base Address Registers from I/O or Memory types.
 *
 * The bus-centric address set in this BAR register is not necessarily accessible from the CPU,
 * thus must be translated by using the PCI Express Controller memory regions translation
 * ranges to permit mapping from the CPU.
 *
 * @param dev PCI Express Controller device pointer
 * @param bdf PCI(e) endpoint
 * @param mem True if the BAR is of memory type
 * @param mem64 True if the BAR is of 64bit memory type
 * @param bar_bus_addr bus-centric address written in the BAR register
 * @param bar_addr CPU-centric address translated from the bus-centric address
 * @return True if translation was possible, False if translation failed
 */
static inline bool pcie_ctrl_region_translate(const struct pci_driver *pdrv, bool mem64, uintptr_t bar_bus_addr,
					  uintptr_t *bar_addr)
{
	const struct pcie_ctrl_driver_api *api =
		(const struct pcie_ctrl_driver_api *)pdrv->api;

	if (!api->region_translate) {
		*bar_addr = bar_bus_addr;
		return true;
	} else {
		return api->region_translate(pdrv, mem64, bar_bus_addr, bar_addr);
	}
}

int pcie_dw_init(void *pcie, struct rk_pcie_cfg *pcie_ranges_cfg,
							struct pcie_dw_data *pcie_data);
void pcie_dw_api_conf(void *pci_drv);

#endif