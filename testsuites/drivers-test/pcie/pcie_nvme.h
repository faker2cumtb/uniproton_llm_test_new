
typedef uint32_t io_port_t;
typedef uintptr_t mm_reg_t;
typedef uintptr_t mem_addr_t;

#define nvme_mmio_offsetof(reg)						\
	offsetof(struct nvme_registers, reg)

#define nvme_mmio_read_4(b_a, reg)					\
	reg_read32((mm_reg_t)b_a +  nvme_mmio_offsetof(reg))

#define nvme_mmio_write_4(b_a, reg, val)				\
	reg_write32(val, (mm_reg_t)b_a + nvme_mmio_offsetof(reg))

#define nvme_mmio_write_8(b_a, reg, val)				\
	do {								\
		reg_write32(val & 0xFFFFFFFF,				\
			    (mm_reg_t)b_a + nvme_mmio_offsetof(reg));	\
		reg_write32((val & 0xFFFFFFFF00000000ULL) >> 32,	\
			    (mm_reg_t)b_a + nvme_mmio_offsetof(reg) + 4); \
	} while (0)

/* Many items are expressed in terms of power of two times MPS */
#define NVME_MPS_SHIFT			12

/* Register field definitions */
#define NVME_CAP_LO_REG_MQES_SHIFT	(0)
#define NVME_CAP_LO_REG_MQES_MASK	(0xFFFF)
#define NVME_CAP_LO_REG_CQR_SHIFT	(16)
#define NVME_CAP_LO_REG_CQR_MASK	(0x1)
#define NVME_CAP_LO_REG_AMS_SHIFT	(17)
#define NVME_CAP_LO_REG_AMS_MASK	(0x3)
#define NVME_CAP_LO_REG_TO_SHIFT	(24)
#define NVME_CAP_LO_REG_TO_MASK		(0xFF)
#define NVME_CAP_LO_MQES(x)						\
	(((x) >> NVME_CAP_LO_REG_MQES_SHIFT) & NVME_CAP_LO_REG_MQES_MASK)
#define NVME_CAP_LO_CQR(x)						\
	(((x) >> NVME_CAP_LO_REG_CQR_SHIFT) & NVME_CAP_LO_REG_CQR_MASK)
#define NVME_CAP_LO_AMS(x)						\
	(((x) >> NVME_CAP_LO_REG_AMS_SHIFT) & NVME_CAP_LO_REG_AMS_MASK)
#define NVME_CAP_LO_TO(x)						\
	(((x) >> NVME_CAP_LO_REG_TO_SHIFT) & NVME_CAP_LO_REG_TO_MASK)

#define NVME_CAP_HI_REG_DSTRD_SHIFT	(0)
#define NVME_CAP_HI_REG_DSTRD_MASK	(0xF)
#define NVME_CAP_HI_REG_NSSRS_SHIFT	(4)
#define NVME_CAP_HI_REG_NSSRS_MASK	(0x1)
#define NVME_CAP_HI_REG_CSS_SHIFT	(5)
#define NVME_CAP_HI_REG_CSS_MASK	(0xff)
#define NVME_CAP_HI_REG_CSS_NVM_SHIFT	(5)
#define NVME_CAP_HI_REG_CSS_NVM_MASK	(0x1)
#define NVME_CAP_HI_REG_BPS_SHIFT	(13)
#define NVME_CAP_HI_REG_BPS_MASK	(0x1)
#define NVME_CAP_HI_REG_MPSMIN_SHIFT	(16)
#define NVME_CAP_HI_REG_MPSMIN_MASK	(0xF)
#define NVME_CAP_HI_REG_MPSMAX_SHIFT	(20)
#define NVME_CAP_HI_REG_MPSMAX_MASK	(0xF)
#define NVME_CAP_HI_REG_PMRS_SHIFT	(24)
#define NVME_CAP_HI_REG_PMRS_MASK	(0x1)
#define NVME_CAP_HI_REG_CMBS_SHIFT	(25)
#define NVME_CAP_HI_REG_CMBS_MASK	(0x1)
#define NVME_CAP_HI_DSTRD(x)						\
	(((x) >> NVME_CAP_HI_REG_DSTRD_SHIFT) & NVME_CAP_HI_REG_DSTRD_MASK)
#define NVME_CAP_HI_NSSRS(x)						\
	(((x) >> NVME_CAP_HI_REG_NSSRS_SHIFT) & NVME_CAP_HI_REG_NSSRS_MASK)
#define NVME_CAP_HI_CSS(x)						\
	(((x) >> NVME_CAP_HI_REG_CSS_SHIFT) & NVME_CAP_HI_REG_CSS_MASK)
#define NVME_CAP_HI_CSS_NVM(x)						\
	(((x) >> NVME_CAP_HI_REG_CSS_NVM_SHIFT) & NVME_CAP_HI_REG_CSS_NVM_MASK)
#define NVME_CAP_HI_BPS(x)						\
	(((x) >> NVME_CAP_HI_REG_BPS_SHIFT) & NVME_CAP_HI_REG_BPS_MASK)
#define NVME_CAP_HI_MPSMIN(x)						\
	(((x) >> NVME_CAP_HI_REG_MPSMIN_SHIFT) & NVME_CAP_HI_REG_MPSMIN_MASK)
#define NVME_CAP_HI_MPSMAX(x)						\
	(((x) >> NVME_CAP_HI_REG_MPSMAX_SHIFT) & NVME_CAP_HI_REG_MPSMAX_MASK)
#define NVME_CAP_HI_PMRS(x)						\
	(((x) >> NVME_CAP_HI_REG_PMRS_SHIFT) & NVME_CAP_HI_REG_PMRS_MASK)
#define NVME_CAP_HI_CMBS(x)						\
	(((x) >> NVME_CAP_HI_REG_CMBS_SHIFT) & NVME_CAP_HI_REG_CMBS_MASK)

#define NVME_CC_REG_EN_SHIFT		(0)
#define NVME_CC_REG_EN_MASK		(0x1)
#define NVME_CC_REG_CSS_SHIFT		(4)
#define NVME_CC_REG_CSS_MASK		(0x7)
#define NVME_CC_REG_MPS_SHIFT		(7)
#define NVME_CC_REG_MPS_MASK		(0xF)
#define NVME_CC_REG_AMS_SHIFT		(11)
#define NVME_CC_REG_AMS_MASK		(0x7)
#define NVME_CC_REG_SHN_SHIFT		(14)
#define NVME_CC_REG_SHN_MASK		(0x3)
#define NVME_CC_REG_IOSQES_SHIFT	(16)
#define NVME_CC_REG_IOSQES_MASK		(0xF)
#define NVME_CC_REG_IOCQES_SHIFT	(20)
#define NVME_CC_REG_IOCQES_MASK		(0xF)

#define NVME_CSTS_REG_RDY_SHIFT		(0)
#define NVME_CSTS_REG_RDY_MASK		(0x1)
#define NVME_CSTS_REG_CFS_SHIFT		(1)
#define NVME_CSTS_REG_CFS_MASK		(0x1)
#define NVME_CSTS_REG_SHST_SHIFT	(2)
#define NVME_CSTS_REG_SHST_MASK		(0x3)
#define NVME_CSTS_REG_NVSRO_SHIFT	(4)
#define NVME_CSTS_REG_NVSRO_MASK	(0x1)
#define NVME_CSTS_REG_PP_SHIFT		(5)
#define NVME_CSTS_REG_PP_MASK		(0x1)

#define NVME_CSTS_GET_SHST(csts)					\
	(((csts) >> NVME_CSTS_REG_SHST_SHIFT) & NVME_CSTS_REG_SHST_MASK)

#define NVME_AQA_REG_ASQS_SHIFT		(0)
#define NVME_AQA_REG_ASQS_MASK		(0xFFF)
#define NVME_AQA_REG_ACQS_SHIFT		(16)
#define NVME_AQA_REG_ACQS_MASK		(0xFFF)

#define NVME_PMRCAP_REG_RDS_SHIFT	(3)
#define NVME_PMRCAP_REG_RDS_MASK	(0x1)
#define NVME_PMRCAP_REG_WDS_SHIFT	(4)
#define NVME_PMRCAP_REG_WDS_MASK	(0x1)
#define NVME_PMRCAP_REG_BIR_SHIFT	(5)
#define NVME_PMRCAP_REG_BIR_MASK	(0x7)
#define NVME_PMRCAP_REG_PMRTU_SHIFT	(8)
#define NVME_PMRCAP_REG_PMRTU_MASK	(0x3)
#define NVME_PMRCAP_REG_PMRWBM_SHIFT	(10)
#define NVME_PMRCAP_REG_PMRWBM_MASK	(0xf)
#define NVME_PMRCAP_REG_PMRTO_SHIFT	(16)
#define NVME_PMRCAP_REG_PMRTO_MASK	(0xff)
#define NVME_PMRCAP_REG_CMSS_SHIFT	(24)
#define NVME_PMRCAP_REG_CMSS_MASK	(0x1)

#define NVME_PMRCAP_RDS(x)						\
	(((x) >> NVME_PMRCAP_REG_RDS_SHIFT) & NVME_PMRCAP_REG_RDS_MASK)
#define NVME_PMRCAP_WDS(x)						\
	(((x) >> NVME_PMRCAP_REG_WDS_SHIFT) & NVME_PMRCAP_REG_WDS_MASK)
#define NVME_PMRCAP_BIR(x)						\
	(((x) >> NVME_PMRCAP_REG_BIR_SHIFT) & NVME_PMRCAP_REG_BIR_MASK)
#define NVME_PMRCAP_PMRTU(x)						\
	(((x) >> NVME_PMRCAP_REG_PMRTU_SHIFT) & NVME_PMRCAP_REG_PMRTU_MASK)
#define NVME_PMRCAP_PMRWBM(x)						\
	(((x) >> NVME_PMRCAP_REG_PMRWBM_SHIFT) & NVME_PMRCAP_REG_PMRWBM_MASK)
#define NVME_PMRCAP_PMRTO(x)						\
	(((x) >> NVME_PMRCAP_REG_PMRTO_SHIFT) & NVME_PMRCAP_REG_PMRTO_MASK)
#define NVME_PMRCAP_CMSS(x)						\
	(((x) >> NVME_PMRCAP_REG_CMSS_SHIFT) & NVME_PMRCAP_REG_CMSS_MASK)

#define NVME_MAJOR(r)			(((r) >> 16) & 0xffff)
#define NVME_MINOR(r)			(((r) >> 8) & 0xff)

struct nvme_registers {
	uint32_t	cap_lo; /* controller capabilities */
	uint32_t	cap_hi;
	uint32_t	vs;	/* version */
	uint32_t	intms;	/* interrupt mask set */
	uint32_t	intmc;	/* interrupt mask clear */
	uint32_t	cc;	/* controller configuration */
	uint32_t	reserved1;
	uint32_t	csts;	/* controller status */
	uint32_t	nssr;	/* NVM Subsystem Reset */
	uint32_t	aqa;	/* admin queue attributes */
	uint64_t	asq;	/* admin submission queue base addr */
	uint64_t	acq;	/* admin completion queue base addr */
	uint32_t	cmbloc;	/* Controller Memory Buffer Location */
	uint32_t	cmbsz;	/* Controller Memory Buffer Size */
	uint32_t	bpinfo;	/* Boot Partition Information */
	uint32_t	bprsel;	/* Boot Partition Read Select */
	uint64_t	bpmbl;	/* Boot Partition Memory Buffer Location */
	uint64_t	cmbmsc;	/* Controller Memory Buffer Memory Space Control */
	uint32_t	cmbsts;	/* Controller Memory Buffer Status */
	uint8_t		reserved3[3492]; /* 5Ch - DFFh */
	uint32_t	pmrcap;	/* Persistent Memory Capabilities */
	uint32_t	pmrctl;	/* Persistent Memory Region Control */
	uint32_t	pmrsts;	/* Persistent Memory Region Status */
	uint32_t	pmrebs;	/* Persistent Memory Region Elasticity Buffer Size */
	uint32_t	pmrswtp; /* Persistent Memory Region Sustained Write Throughput */
	uint32_t	pmrmsc_lo; /* Persistent Memory Region Controller Memory Space Control */
	uint32_t	pmrmsc_hi;
	uint8_t		reserved4[484]; /* E1Ch - FFFh */
	struct {
		uint32_t	sq_tdbl; /* submission queue tail doorbell */
		uint32_t	cq_hdbl; /* completion queue head doorbell */
	} doorbell[1];
};
