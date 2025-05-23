#ifndef CPU_CONFIG_H
#define CPU_CONFIG_H

#include "prt_cache_external.h"

#define MMU_IMAGE_ADDR        0x4A000000ULL

#define MMU_OPENAMP_ADDR      0x60000000ULL
#define OPENAMP_SHM_SIZE      0x1800000U          //24M

#define MMU_GIC_ADDR          0xFD400000ULL

#define MMU_PMU_GRF_ADDR      0xFDC20000ULL
#define MMU_SYS_GRF_ADDR      0xFDC60000ULL

#define MMU_PMU_CRU_ADDR      0xFDD00000ULL
#define MMU_CRU_ADDR          0xFDD20000ULL

#define MMU_GPIO0_ADDR        0xFDD60000ULL
#define MMU_GPIO1_ADDR        0xFE740000ULL
#define MMU_GPIO2_ADDR        0xFE750000ULL
#define MMU_GPIO3_ADDR        0xFE760000ULL
#define MMU_GPIO4_ADDR        0xFE770000ULL

#define MMU_UART1_ADDR        0xFE650000ULL
#define MMU_UART2_ADDR        0xFE660000ULL
#define MMU_UART3_ADDR        0xFE670000ULL
#define MMU_UART4_ADDR        0xFE680000ULL
#define MMU_UART5_ADDR        0xFE690000ULL
#define MMU_UART6_ADDR        0xFE6A0000ULL
#define MMU_UART7_ADDR        0xFE6B0000ULL
#define MMU_UART8_ADDR        0xFE6C0000ULL
#define MMU_UART9_ADDR        0xFE6D0000ULL

#define MMU_CAN0_ADDR         0xFE570000ULL
#define MMU_CAN1_ADDR         0xFE580000ULL
#define MMU_CAN2_ADDR         0xFE590000ULL

#define MMU_I2C0_ADDR         0xFDD40000ULL
#define MMU_I2C1_ADDR         0xFE5A0000ULL
#define MMU_I2C2_ADDR         0xFE5B0000ULL
#define MMU_I2C3_ADDR         0xFE5C0000ULL
#define MMU_I2C4_ADDR         0xFE5D0000ULL
#define MMU_I2C5_ADDR         0xFE5E0000ULL

#define MMU_TIMER_ADDR        0xFE5F0000ULL

#define MMU_SPI0_ADDR         0xFE610000ULL
#define MMU_SPI1_ADDR         0xFE620000ULL
#define MMU_SPI2_ADDR         0xFE630000ULL
#define MMU_SPI3_ADDR         0xFE640000ULL

#define MMU_GMAC0_ADDR        0xFE2A0000ULL
#define MMU_GMAC1_ADDR        0xFE010000ULL

#define TEST_CLK_INT               30

#define OS_GIC_VER                 3
#define SICR_ADDR_OFFSET_PER_CORE  0x20000U
#define GIC_REG_BASE_ADDR          0xFD400000ULL

#define GICD_CTLR_S_ADDR           (GIC_REG_BASE_ADDR + 0x0000U)
#define GICD_IGROUPN_ADDR          (GIC_REG_BASE_ADDR + 0x0080U)
#define GICD_ISENABLER0_ADDR       (GIC_REG_BASE_ADDR + 0x0100U)
#define GICD_ICENABLER0_ADDR       (GIC_REG_BASE_ADDR + 0x0180U)
#define GICD_IPRIORITYN_ADDR       (GIC_REG_BASE_ADDR + 0x0400U)
#define GICD_IGRPMODRN_ADDR        (GIC_REG_BASE_ADDR + 0x0D00U)
#define GICD_IROUTER               (GIC_REG_BASE_ADDR + 0x6100U)

#define GICR_BASE0                 (GIC_REG_BASE_ADDR + 0x60000U)
#define GICR_BASE1                 (GIC_REG_BASE_ADDR + 0x70000U)

#define GICR_CTRL_ADDR             (GICR_BASE0 + 0x0000U)
#define GICR_WAKER_ADDR            (GICR_BASE0 + 0x0014U)

#define GICR_IGROUPR0_ADDR         (GICR_BASE1 + 0x0080U)
#define GICR_ISENABLER0_ADDR       (GICR_BASE1 + 0x0100U)
#define GICR_ICENABLER0_ADDR       (GICR_BASE1 + 0x0180U)
#define GICR_IGRPMODR0_ADDR        (GICR_BASE1 + 0x0D00U)

#define MAX_INT_NUM                283
#define MIN_GIC_SPI_NUM            32
#define SICD_IGROUP_INT_NUM        32
#define SICD_REG_SIZE              4

#define GROUP_MAX_BPR              0x7U
#define GROUP0_BP                  0
#define GROUP1_BP                  0

#define PRIO_MASK_LEVEL            0xFFU

#define ICC_SRE_EL1                S3_0_C12_C12_5
#define ICC_BPR0_EL1               S3_0_C12_C8_3
#define ICC_BPR1_EL1               S3_0_C12_C12_3
#define ICC_IGRPEN1_EL1            S3_0_C12_C12_7
#define ICC_PMR_EL1                S3_0_C4_C6_0

#define PARAS_TO_STRING(x...)      #x
#define REG_ALIAS(x...)            PARAS_TO_STRING(x)

#define GIC_REG_READ(addr)         (*(volatile U32 *)((uintptr_t)(addr)))
#define GIC_REG_WRITE(addr, data)  (*(volatile U32 *)((uintptr_t)(addr)) = (U32)(data))

#endif
