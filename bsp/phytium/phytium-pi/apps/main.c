#include <prt_buildef.h>
#include <prt_sys.h>
#include <print.h>
#include "demo.h"

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
    PRT_Printf("Hello, %s\n", PRT_SysGetOsVersion());
    PRT_Printf("Phytium Pi CEK8903-V2.2 @ %s %s\n", __DATE__, __TIME__);

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
    FUNC_CALL(spi_loopback_demo);
#elif defined(DEMO_SMP)
    FUNC_CALL(smp_demo);
#elif defined(DEMO_MAC)
    FUNC_CALL(mac_demo);
#elif defined(DEMO_LWIP_UDP)
#if defined(OS_OPTION_PROXY) && !defined(OS_SUPPORT_NET)
#error "OS_SUPPORT_NET not defined!"
#endif
    FUNC_CALL(Lwip_Udp_Demo);
#elif defined(DEMO_LWIP_TCP)
#if defined(OS_OPTION_PROXY) && !defined(OS_SUPPORT_NET)
#error "OS_SUPPORT_NET not defined!"
#endif
    FUNC_CALL(Lwip_Tcp_Demo);
#elif defined(DEMO_ETHERCAT)
    FUNC_CALL(ethercat_demo);
#elif defined(DEMO_CANOPEN)
    FUNC_CALL(canopen_demo);
#elif defined(DEMO_MODBUS_CLI_RTU)
    FUNC_CALL(Modbus_Client_RTU_Demo);
#elif defined(DEMO_MODBUS_SRV_RTU)
    FUNC_CALL(Modbus_Server_RTU_Demo);
#elif defined(DEMO_MODBUS_CLI_TCP)
    FUNC_CALL(Modbus_Client_TCP_Demo);
#elif defined(DEMO_MODBUS_SRV_TCP)
    FUNC_CALL(Modbus_Server_TCP_Demo);
#elif defined(DEMO_POWEROFF_PROTECT)
    FUNC_CALL(demo_poweroff_protect);
#elif defined(DEMO_MBEDTLS) && defined(SECURITY_MBEDTLS)
    FUNC_CALL(mbedtls_demo);
#elif defined(DEMO_VERITY)
    FUNC_CALL(verify_demo);
#elif defined(DEMO_PROXY_UDP) && defined(OS_OPTION_PROXY)
    FUNC_CALL(proxy_udp_demo);
#elif defined(DEMO_LOG)
    FUNC_CALL(log_demo);
#endif
}
