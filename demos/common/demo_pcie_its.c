#include "prt_gic_its.h"
#include "prt_hwi.h"
#include "cpu_config.h"
#include "prt_task.h"

#define TEST_LPI_IRQ_9000                       9000U
#define TEST_LPI_IRQ_9100                       9100U
#define TEST_LPI_IRQ_9200                       9200U

// device information
#define DEVICE_NUMBER                           3U

#define GITS_COLLECT_ID                         0U

struct its_device_s g_its_device = {0};

void its_demo_lpi_handler_9000(uintptr_t para)
{
    PRT_Printf("[SUCCESS] reccceived lpi 9000.\n");
}

void its_demo_lpi_handler_9100(uintptr_t para)
{
    PRT_Printf("[SUCCESS] reccceived lpi 9100.\n");
}

void its_demo_lpi_handler_9200(uintptr_t para)
{
    PRT_Printf("[SUCCESS] reccceived lpi 9200.\n");
}

U32 its_test_demo_apply_irq(U32 hwirq, void *handler_function)
{
    U32 ret = PRT_HwiSetAttr(hwirq, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(hwirq, (HwiProcFunc)handler_function, 0);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_HwiEnable(hwirq);
    if (ret != OS_OK) {
        return ret;
    }
    return OS_OK;
}

struct its_device_info_s g_device_list[DEVICE_NUMBER] = {
    {0x1U, TEST_LPI_IRQ_9000, 2U},
    {0x2U, TEST_LPI_IRQ_9100, 2U},
    {0x3U, TEST_LPI_IRQ_9200, 2U},
};

void pcie_its_demo()
{
    PRT_Printf("[TEST] its test demo enter.\n");

    /*initialize its device*/
    struct its_device_s *its_device = &g_its_device;
    its_device_init(its_device);

    /*apply irq and regist handler function*/
    its_test_demo_apply_irq(TEST_LPI_IRQ_9000, its_demo_lpi_handler_9000);
    its_test_demo_apply_irq(TEST_LPI_IRQ_9100, its_demo_lpi_handler_9100);
    its_test_demo_apply_irq(TEST_LPI_IRQ_9200, its_demo_lpi_handler_9200);

    /*add device info to device list*/
    its_device_info_init(its_device, g_device_list, DEVICE_NUMBER);

    /*MAPC command*/
    struct redist_addr_info_s gicr_addr = {0};
    gicr_addr.phy_addr = GICR_BASE0;
    its_map_icid_to_redist(its_device, &gicr_addr, GITS_COLLECT_ID);

    /*create device table and ITT table according to device info*/
    its_setup_device_table(its_device, GITS_COLLECT_ID);

    /*its enable irq*/
    its_set_irq_enable(its_device, TEST_LPI_IRQ_9000);
    its_set_irq_enable(its_device, TEST_LPI_IRQ_9100);
    its_set_irq_enable(its_device, TEST_LPI_IRQ_9200);

    U32 n = 0;
    while (n < 10) {
        PRT_Printf("[TEST %u]\n", n);
        PRT_TaskDelay(50);

        /*its pend irq*/
        its_set_irq_pending(its_device, TEST_LPI_IRQ_9000);
        PRT_TaskDelay(50);
        its_set_irq_pending(its_device, TEST_LPI_IRQ_9100);
        PRT_TaskDelay(50);
        its_set_irq_pending(its_device, TEST_LPI_IRQ_9200);
        n++;
    }
}