#include "cpu_config.h"
#include "prt_mmu_external.h"

extern U32 data_copy_start;
extern U64 __protect_len;
extern U64 __non_protect_len;

mmu_mmap_region_s g_mem_map_info[] = {
    {
        .virt = MMU_IMAGE_ADDR,
        .phys = MMU_IMAGE_ADDR,
        .size = (U64)&__protect_len,
        .max_level = 0x3,
#ifdef OS_GDB_STUB
        .attrs = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RWX,
#else
        .attrs = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RX,
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
        .virt = MMU_GIC_ADDR,
        .phys = MMU_GIC_ADDR,
        .size = 0x1000000,
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

#if defined(MMU_IOPAD_ADDR)
    {
        .virt = MMU_IOPAD_ADDR,
        .phys = MMU_IOPAD_ADDR,
        .size = 0x2000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO0_ADDR)
    {
        .virt = MMU_GPIO0_ADDR,
        .phys = MMU_GPIO0_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO1_ADDR)
    {
        .virt = MMU_GPIO1_ADDR,
        .phys = MMU_GPIO1_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO2_ADDR)
    {
        .virt = MMU_GPIO2_ADDR,
        .phys = MMU_GPIO2_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO3_ADDR)
    {
        .virt = MMU_GPIO3_ADDR,
        .phys = MMU_GPIO3_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO4_ADDR)
    {
        .virt = MMU_GPIO4_ADDR,
        .phys = MMU_GPIO4_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO5_ADDR)
    {
        .virt = MMU_GPIO5_ADDR,
        .phys = MMU_GPIO5_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO0_ADDR)
    {
        .virt = MMU_MIO0_ADDR,
        .phys = MMU_MIO0_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO1_ADDR)
    {
        .virt = MMU_MIO1_ADDR,
        .phys = MMU_MIO1_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO2_ADDR)
    {
        .virt = MMU_MIO2_ADDR,
        .phys = MMU_MIO2_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO3_ADDR)
    {
        .virt = MMU_MIO3_ADDR,
        .phys = MMU_MIO3_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO4_ADDR)
    {
        .virt = MMU_MIO4_ADDR,
        .phys = MMU_MIO4_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO5_ADDR)
    {
        .virt = MMU_MIO5_ADDR,
        .phys = MMU_MIO5_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO6_ADDR)
    {
        .virt = MMU_MIO6_ADDR,
        .phys = MMU_MIO6_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO7_ADDR)
    {
        .virt = MMU_MIO7_ADDR,
        .phys = MMU_MIO7_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO8_ADDR)
    {
        .virt = MMU_MIO8_ADDR,
        .phys = MMU_MIO8_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO9_ADDR)
    {
        .virt = MMU_MIO9_ADDR,
        .phys = MMU_MIO9_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO10_ADDR)
    {
        .virt = MMU_MIO10_ADDR,
        .phys = MMU_MIO10_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO11_ADDR)
    {
        .virt = MMU_MIO11_ADDR,
        .phys = MMU_MIO11_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO12_ADDR)
    {
        .virt = MMU_MIO12_ADDR,
        .phys = MMU_MIO12_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO13_ADDR)
    {
        .virt = MMU_MIO13_ADDR,
        .phys = MMU_MIO13_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO14_ADDR)
    {
        .virt = MMU_MIO14_ADDR,
        .phys = MMU_MIO14_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_MIO15_ADDR)
    {
        .virt = MMU_MIO15_ADDR,
        .phys = MMU_MIO15_ADDR,
        .size = 0x1200,
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

#if defined(MMU_I2C0_ADDR)
    {
        .virt = MMU_I2C0_ADDR,
        .phys = MMU_I2C0_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C1_ADDR)
    {
        .virt = MMU_I2C1_ADDR,
        .phys = MMU_I2C1_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C2_ADDR)
    {
        .virt = MMU_I2C2_ADDR,
        .phys = MMU_I2C2_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI0_ADDR)
    {
        .virt = MMU_SPI0_ADDR,
        .phys = MMU_SPI0_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI1_ADDR)
    {
        .virt = MMU_SPI1_ADDR,
        .phys = MMU_SPI1_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI2_ADDR)
    {
        .virt = MMU_SPI2_ADDR,
        .phys = MMU_SPI2_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI3_ADDR)
    {
        .virt = MMU_SPI3_ADDR,
        .phys = MMU_SPI3_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CAN0_ADDR)
    {
        .virt = MMU_CAN0_ADDR,
        .phys = MMU_CAN0_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CAN1_ADDR)
    {
        .virt = MMU_CAN1_ADDR,
        .phys = MMU_CAN1_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_TIMER0_ADDR)
    {
        .virt = MMU_TIMER0_ADDR,
        .phys = MMU_TIMER0_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_TIMER1_ADDR)
    {
        .virt = MMU_TIMER1_ADDR,
        .phys = MMU_TIMER1_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_TIMER2_ADDR)
    {
        .virt = MMU_TIMER2_ADDR,
        .phys = MMU_TIMER2_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_TIMER3_ADDR)
    {
        .virt = MMU_TIMER3_ADDR,
        .phys = MMU_TIMER3_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_FXMAC0_ADDR)
    {
        .virt = MMU_FXMAC0_ADDR,
        .phys = MMU_FXMAC0_ADDR,
        .size = 0x1200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_FXMAC1_ADDR)
    {
        .virt = MMU_FXMAC1_ADDR,
        .phys = MMU_FXMAC1_ADDR,
        .size = 0xb00,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_FXMAC2_ADDR)
    {
        .virt = MMU_FXMAC2_ADDR,
        .phys = MMU_FXMAC2_ADDR,
        .size = 0xb00,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_FXMAC3_ADDR)
    {
        .virt = MMU_FXMAC3_ADDR,
        .phys = MMU_FXMAC3_ADDR,
        .size = 0xb00,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_LOCALBUS_ADDR)
    {
        .virt = MMU_LOCALBUS_ADDR,
        .phys = MMU_LOCALBUS_ADDR,
        .size = 0x300,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
    {
        .virt = 0x10000000,
        .phys = 0x10000000,
        .size = 0x800,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
    {
        .virt = 0x11000000,
        .phys = 0x11000000,
        .size = 0x800,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
    {
        .virt = 0x11800000,
        .phys = 0x11800000,
        .size = 0x800,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif
};

size_t mmu_table_size = sizeof(g_mem_map_info) / sizeof(mmu_mmap_region_s);
