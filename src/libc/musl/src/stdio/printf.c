#include <stdio.h>
#include <stdarg.h>

// int printf(const char *restrict fmt, ...)
// {
//     int ret;
//     va_list ap;
//     va_start(ap, fmt);
//     ret = vfprintf(stdout, fmt, ap);
//     va_end(ap);
//     return ret;
// }

int printf(const char *__restrict format, ...)
{
    va_list vaList;
    int count;
    
    va_start(vaList, format);
    count = TestPrintf(format, vaList);
    va_end(vaList);
    
    return count;
}

