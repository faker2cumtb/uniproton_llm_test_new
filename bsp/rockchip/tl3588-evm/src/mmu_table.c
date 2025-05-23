#include "cpu_config.h"
#include "prt_mmu_external.h"

extern U32 data_copy_start;
extern U64 __protect_len;
extern U64 __non_protect_len;

extern U64 g_mmu_page_begin;
extern U64 __mmu_len;

mmu_mmap_region_s g_mem_map_info[] = {
    {
        .virt = MMU_IMAGE_ADDR,
        .phys = MMU_IMAGE_ADDR,
        .size = (U64)&__protect_len,
        .max_level = 0x3,
#ifdef OS_GDB_STUB
        .attrs     = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RWX,
#else
        .attrs     = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RX,
#endif
    },
    {
        .virt = (U64)&data_copy_start,
        .phys = (U64)&data_copy_start,
        .size = (U64)&__non_protect_len,
        .max_level = 0x3,
        .attrs = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RWX,
    },
    {
        .virt = MMU_GMAC_DESC_ADDR,
        .phys = MMU_GMAC_DESC_ADDR,
        .size = 0x8000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_UNCACHE_SHARE | MMU_ACCESS_RWX,
    },
    {
        .virt = (U64)&g_mmu_page_begin,
        .phys = (U64)&g_mmu_page_begin,
        .size = (U64)&__mmu_len,
        .max_level = 0x2,
        .attrs = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RWX,
    },
    {
        .virt = MMU_GIC_ADDR,
        .phys = MMU_GIC_ADDR,
        .size = 0x1000000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
    {
        .virt = MMU_GIC_ITS,
        .phys = MMU_GIC_ITS,
        .size = 0x20000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
    
#if defined(MMU_OPENAMP_ADDR)
    {
        .virt = MMU_OPENAMP_ADDR,
        .phys = MMU_OPENAMP_ADDR,
        .size = OPENAMP_SHM_SIZE,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PCIE3x4L_ADDR)
    {
        .virt = MMU_PCIE3x4L_ADDR,
        .phys = MMU_PCIE3x4L_ADDR,
        .size = 0x1000000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PCIE3X4L_APB_ADDR)
    {
        .virt = MMU_PCIE3X4L_APB_ADDR,
        .phys = MMU_PCIE3X4L_APB_ADDR,
        .size = 0x10000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SYS_GRF_ADDR)
    {
        .virt = MMU_SYS_GRF_ADDR,
        .phys = MMU_SYS_GRF_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PHP_GRF_ADDR)
    {
        .virt = MMU_PHP_GRF_ADDR,
        .phys = MMU_PHP_GRF_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PCIE3PHY_GRF_ADDR)
    {
        .virt = MMU_PCIE3PHY_GRF_ADDR,
        .phys = MMU_PCIE3PHY_GRF_ADDR,
        .size = 0x4000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PMU1_IOC_ADDR)
    {
        .virt = MMU_PMU1_IOC_ADDR,
        .phys = MMU_PMU1_IOC_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PMU2_IOC_ADDR)
    {
        .virt = MMU_PMU2_IOC_ADDR,
        .phys = MMU_PMU2_IOC_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_BUS_IOC_ADDR)
    {
        .virt = MMU_BUS_IOC_ADDR,
        .phys = MMU_BUS_IOC_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CRU_ADDR)
    {
        .virt = MMU_CRU_ADDR,
        .phys = MMU_CRU_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PMU1_CRU_ADDR)
    {
        .virt = MMU_PMU1_CRU_ADDR,
        .phys = MMU_PMU1_CRU_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART0_ADDR)
    {
        .virt = MMU_UART0_ADDR,
        .phys = MMU_UART0_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART1_ADDR)
    {
        .virt = MMU_UART1_ADDR,
        .phys = MMU_UART1_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART2_ADDR)
    {
        .virt = MMU_UART2_ADDR,
        .phys = MMU_UART2_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART3_ADDR)
    {
        .virt = MMU_UART3_ADDR,
        .phys = MMU_UART3_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART4_ADDR)
    {
        .virt = MMU_UART4_ADDR,
        .phys = MMU_UART4_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART5_ADDR)
    {
        .virt = MMU_UART5_ADDR,
        .phys = MMU_UART5_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART6_ADDR)
    {
        .virt = MMU_UART6_ADDR,
        .phys = MMU_UART6_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART7_ADDR)
    {
        .virt = MMU_UART7_ADDR,
        .phys = MMU_UART7_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART8_ADDR)
    {
        .virt = MMU_UART8_ADDR,
        .phys = MMU_UART8_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART9_ADDR)
    {
        .virt = MMU_UART9_ADDR,
        .phys = MMU_UART9_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO0_ADDR)
    {
        .virt = MMU_GPIO0_ADDR,
        .phys = MMU_GPIO0_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO1_ADDR)
    {
        .virt = MMU_GPIO1_ADDR,
        .phys = MMU_GPIO1_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO2_ADDR)
    {
        .virt = MMU_GPIO2_ADDR,
        .phys = MMU_GPIO2_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO3_ADDR)
    {
        .virt = MMU_GPIO3_ADDR,
        .phys = MMU_GPIO3_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO4_ADDR)
    {
        .virt = MMU_GPIO4_ADDR,
        .phys = MMU_GPIO4_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CAN0_ADDR)
    {
        .virt = MMU_CAN0_ADDR,
        .phys = MMU_CAN0_ADDR,
        .size = 0x600,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CAN1_ADDR)
    {
        .virt = MMU_CAN1_ADDR,
        .phys = MMU_CAN1_ADDR,
        .size = 0x600,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CAN2_ADDR)
    {
        .virt = MMU_CAN2_ADDR,
        .phys = MMU_CAN2_ADDR,
        .size = 0x600,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_TIMER_ADDR)
    {
        .virt = MMU_TIMER_ADDR,
        .phys = MMU_TIMER_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
    {
        .virt = MMU_TIMER_ADDR + 0x8000,
        .phys = MMU_TIMER_ADDR + 0x8000,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI0_ADDR)
    {
        .virt = MMU_SPI0_ADDR,
        .phys = MMU_SPI0_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI1_ADDR)
    {
        .virt = MMU_SPI1_ADDR,
        .phys = MMU_SPI1_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI2_ADDR)
    {
        .virt = MMU_SPI2_ADDR,
        .phys = MMU_SPI2_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI3_ADDR)
    {
        .virt = MMU_SPI3_ADDR,
        .phys = MMU_SPI3_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI4_ADDR)
    {
        .virt = MMU_SPI4_ADDR,
        .phys = MMU_SPI4_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C0_ADDR)
    {
        .virt = MMU_I2C0_ADDR,
        .phys = MMU_I2C0_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C1_ADDR)
    {
        .virt = MMU_I2C1_ADDR,
        .phys = MMU_I2C1_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C2_ADDR)
    {
        .virt = MMU_I2C2_ADDR,
        .phys = MMU_I2C2_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C3_ADDR)
    {
        .virt = MMU_I2C3_ADDR,
        .phys = MMU_I2C3_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C4_ADDR)
    {
        .virt = MMU_I2C4_ADDR,
        .phys = MMU_I2C4_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C5_ADDR)
    {
        .virt = MMU_I2C5_ADDR,
        .phys = MMU_I2C5_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C6_ADDR)
    {
        .virt = MMU_I2C6_ADDR,
        .phys = MMU_I2C6_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C7_ADDR)
    {
        .virt = MMU_I2C7_ADDR,
        .phys = MMU_I2C7_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C8_ADDR)
    {
        .virt = MMU_I2C8_ADDR,
        .phys = MMU_I2C8_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GMAC0_ADDR)
    {
        .virt = MMU_GMAC0_ADDR,
        .phys = MMU_GMAC0_ADDR,
        .size = 0x2000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GMAC1_ADDR)
    {
        .virt = MMU_GMAC1_ADDR,
        .phys = MMU_GMAC1_ADDR,
        .size = 0x2000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PCIE3X4_DBI_ADDR)
    {
        .virt = MMU_PCIE3X4_DBI_ADDR,
        .phys = MMU_PCIE3X4_DBI_ADDR,
        .size = 0x400000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif
};

size_t mmu_table_size = sizeof(g_mem_map_info) / sizeof(mmu_mmap_region_s);
