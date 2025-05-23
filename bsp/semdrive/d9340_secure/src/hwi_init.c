#include "securec.h"
#include "prt_tick.h"
#include "prt_hwi.h"
#include "prt_sys.h"
#include "prt_task.h"
#include "cpu_config.h"
#include "irq_num.h"
#include <stdio.h>


int IsrRegister(U32 intNo, U32 pri)
{
    // num
    U32 bitn, reg, shift;
    U32 *addr;
    U32 priori = 80;
    bitn = intNo / 32U;
    addr = (U32 *)(GICD_ISENABLERn + 4U * bitn);
    reg = *addr;
    *addr = (reg | (0x1U << (intNo % 32U)));

     printf("intNo = %08x, value = %08x\n", addr, *addr);
    // pri


    bitn = intNo / 4U;
    addr = (U32 *)(GICD_IPRIORITYn + 4U * bitn);
    shift = (intNo % 4U) * 8U;
    reg = (*addr) & ~(0xFFU << shift);
    *addr = (reg | priori << shift);

    printf("pri = %08x, value = %08x\n", addr, *addr);
}

void OsGicInitCpuInterface(void)
{
    int i;
    U32 val;

    GIC_REG_WRITE(GICD_CTLR, 1);
    /* disable int */
    for (i = 0; i < IRQ_MAX_INTR_NUM; i += 32) {
        GIC_REG_WRITE(GICD_ICENABLERn + (i/8), 0xFFFFFFFF);
        GIC_REG_WRITE(GICD_ICPENDRn + (i/8), 0xFFFFFFFF);
    }

    /* set SPI cpu to cpu 0 and Non-secure view */
    for (i = 32; i < IRQ_MAX_INTR_NUM; i += 4) {
        GIC_REG_WRITE(GICD_IPRIORITYn + i, 0x80808080);
        GIC_REG_WRITE(GICD_ITARGETSRn + i, 0x01010101);
    }

    GIC_REG_WRITE(GICC_PMR, 0xFF);

    GIC_REG_WRITE(GICC_CTLR, 1);
}

U32 OsHwiInit(void)
{
    OsGicInitCpuInterface();

    return OS_OK;
}
