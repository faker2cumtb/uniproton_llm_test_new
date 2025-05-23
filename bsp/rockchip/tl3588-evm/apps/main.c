#include <prt_buildef.h>
#include <prt_sys.h>
#include <print.h>
#include "demo.h"
#include "mica_service.h"

#define DEMO_TIMER

#define FUNC_CALL(func)                        \
    do                                         \
    {                                          \
        PRT_Printf("%s running ...\n", #func); \
        func();                                \
        PRT_Printf("%s finish.\n", #func);     \
    } while (0)

void apps_entry()
{
    PRT_Printf("\nHello, %s\n", PRT_SysGetOsVersion());
    PRT_Printf("TL3588-EVM @ %s %s\n", __DATE__, __TIME__);
    
#if defined(TESTSUITE_CASE)
    Init(0, 0, 0, 0);
#elif defined(DEMO_GPIO)
    FUNC_CALL(gpio_demo);
#elif defined(DEMO_UART)
    FUNC_CALL(uart_demo);
#elif defined(DEMO_TIMER)
    FUNC_CALL(timer_demo);
#elif defined(DEMO_CAN)
    FUNC_CALL(can_demo);
#elif defined(DEMO_SPI)
    FUNC_CALL(spi_xl2515_demo);
#elif defined(DEMO_I2C)
    FUNC_CALL(i2c_ds1307_demo);
#elif defined(DEMO_MAC)
    FUNC_CALL(mac_demo);
#elif defined(DEMO_CANOPEN)
    FUNC_CALL(canopen_demo);
#elif defined(DEMO_SMP)
    FUNC_CALL(smp_demo);
#elif defined(DEMO_ETHERCAT)
    FUNC_CALL(ethercat_demo);
#elif defined(DEMO_PCIE_ITS)
    FUNC_CALL(pcie_its_demo);
#elif defined(DEMO_PCIE)
    FUNC_CALL(pcie_demo);
#elif defined(DEMO_LOG)
    FUNC_CALL(log_demo);
#elif defined(DEMO_VERITY)
    FUNC_CALL(verify_demo);
#endif
}
