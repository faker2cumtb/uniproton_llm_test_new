#include <stdarg.h>
#include "prt_typedef.h"
#include "cpu_config.h"
#include "securec.h"
#include "prt_hwi.h"

static unsigned int FtIn32(uintptr_t addr)
{
    return *(volatile unsigned int *)addr;
}

static void FtOut32(uintptr_t addr, unsigned int value)
{
    volatile unsigned int *local_addr = (volatile unsigned int *)addr;
    *local_addr = value;
}

typedef U32 (*PrintFunc)(const char *format, va_list vaList);
#define OS_MAX_SHOW_LEN 0x200
#define FPL011FTR_OFFSET        0x7C
#define FPL011FTR_TXFF 0x2U /*  Transmit FIFO full. */
#define FUART10_BASE_ADDR 0xF04D0000
#define FUART_ISTRANSMITFULL(addr) ((FtIn32(addr + FPL011FTR_OFFSET) & (unsigned int)FPL011FTR_TXFF) != FPL011FTR_TXFF)
#define FUART_WRITEREG32(addr, reg_offset, reg_value) FtOut32(addr + (unsigned int)reg_offset, (unsigned int)reg_value)
#define OS_OK       0

void FPl011SendByte(uintptr_t addr, uint8_t byte)
{

    while (FUART_ISTRANSMITFULL(addr))
    {
        ;
    }
    // for(int i=0; i<1000; i++){

    // }
    FUART_WRITEREG32(addr, 0, (unsigned int)byte);
}
static inline void uart_poll_send(unsigned char ch)
{
    FPl011SendByte(FUART10_BASE_ADDR, ch);
}

static inline void TestPutc(unsigned char ch)
{
    uart_poll_send(ch);
    if (ch == '\n') {
        uart_poll_send('\r');
    }
}

int TestPrintf(const char *format, va_list vaList)
{
    int len;
    char buff[100] = {0};
    char *str = buff;
    
    len = vsnprintf(buff, 100, format, vaList);
    if (len == -1) {
        return len;
    }
    
    while (*str != '\0') {
        TestPutc(*str);
        str++;
    }
    
    return OS_OK;
}
unsigned int PRT_Printf(const char *format, ...)
{
    va_list vaList;
    int count;
    
    va_start(vaList, format);
    count = TestPrintf(format, vaList);
    va_end(vaList);
    
    return count;
}

int printf(const char *__restrict format, ...)
{
    va_list vaList;
    int count;
    
    va_start(vaList, format);
    count = TestPrintf(format, vaList);
    va_end(vaList);
    
    return count;
}