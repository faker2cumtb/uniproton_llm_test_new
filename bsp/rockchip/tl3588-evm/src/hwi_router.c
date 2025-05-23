#include "hal_base.h"
#include "cpu_config.h"
#include "hwi_init.h"

#define MPIDR_AFFLVL_MASK (0xffULL)
#define MIN_SPI_ID 32
#define MAX_SPI_ID 1019

#define MPIDR_AFF_MASK (GENMASK(23, 0) | GENMASK(39, 32))

#define MPIDR_AFFLVL(mpidr, aff_level) \
    (((mpidr) >> MPIDR_AFF##aff_level##_SHIFT) & MPIDR_AFFLVL_MASK)

static inline uint64_t get_mpid(void)
{
    uint64_t mpid;
    OS_EMBED_ASM("MRS  %0, MPIDR_EL1" : "=r"(mpid)::"memory", "cc");
    return mpid;
}

/**
 * 描述：将SPI中断路由到本核
 */
void PRT_HwiSetRouter(uint32_t irq)
{
    if ((irq < MIN_SPI_ID) || (irq > MAX_SPI_ID))
    {
        PRT_Printf("irq route only surpport spi irq.\n");
        return;
    }

    OsSicSetGroup(irq, SIC_GROUP_G1NS);

    uint64_t aff3, aff2, aff1, aff0;
    uint64_t mpidr = get_mpid();
    uint64_t value = 0;

    aff3 = MPIDR_AFFLVL(mpidr, 3);
    aff2 = MPIDR_AFFLVL(mpidr, 2);
    aff1 = MPIDR_AFFLVL(mpidr, 1);
    aff0 = MPIDR_AFFLVL(mpidr, 0);

    volatile unsigned long *ptr = GICD_IROUTER + (irq - 32) * 8;
    value = *ptr;

    value |= aff0;
    value |= (aff1 << 8);
    value |= (aff2 << 16);
    value &= (~(((uint64_t)1) << 31));
    value |= (aff3 << 32);
    *ptr = value;
}