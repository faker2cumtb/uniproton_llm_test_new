#include <stdarg.h>
#include <prt_typedef.h>
#include <prt_hwi.h>
#include <cpu_config.h>
#include <securec.h>
#include <hal_base.h>

#define OS_MAX_SHOW_LEN 0x200

#ifdef GUEST_OS_JAILHOUSE

#include <prt_log.h>
int TestPrintf(const char *format, va_list vaList)
{
    int len;
    char buff[OS_MAX_SHOW_LEN] = {0};
    char *str = buff;
    
    len = vsnprintf_s(buff, OS_MAX_SHOW_LEN, OS_MAX_SHOW_LEN, format, vaList);
    if (len == -1) {
        return len;
    }
    
    (void)PRT_Log(OS_LOG_INFO, OS_LOG_F0, buff, len);
    
    return OS_OK;
}

U32 PRT_PrintfInit()
{
    return OS_OK;
}
#else

static inline void uart_poll_send(unsigned char ch)
{
    HAL_UART_SerialOutChar(UART2, ch);
}

U32 PRT_PrintfInit()
{
    return OS_OK;
}

static void TestPutc(unsigned char ch)
{
    uart_poll_send(ch);
    if (ch == '\n')
    {
        uart_poll_send('\r');
    }
}

int TestPrintf(const char *format, va_list vaList)
{
    int len;
    char buff[OS_MAX_SHOW_LEN] = {0};
    char *str = buff;

    len = vsnprintf_s(buff, OS_MAX_SHOW_LEN, OS_MAX_SHOW_LEN, format, vaList);
    if (len == -1)
    {
        return len;
    }

    while (*str != '\0')
    {
        TestPutc(*str);
        str++;
    }

    return OS_OK;
}
#endif

U32 PRT_Printf(const char *format, ...)
{
    va_list vaList;
    S32 count;

    va_start(vaList, format);
    count = TestPrintf(format, vaList);
    va_end(vaList);

    return count;
}
