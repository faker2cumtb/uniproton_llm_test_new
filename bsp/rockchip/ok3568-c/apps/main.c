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
    PRT_Printf("\nHello, %s\n", PRT_SysGetOsVersion());
    PRT_Printf("OK3568-C @ %s %s\n", __DATE__, __TIME__);

#if defined(TESTSUITE_CASE)
    Init(0, 0, 0, 0);
#elif defined(DEMO_GPIO)
    FUNC_CALL(gpio_demo);
#elif defined(DEMO_UART)
    FUNC_CALL(uart_demo);
#elif defined(DEMO_TIMER)
    FUNC_CALL(timer_demo);
#elif defined(DEMO_SMP)
    FUNC_CALL(smp_demo);
#endif
}
