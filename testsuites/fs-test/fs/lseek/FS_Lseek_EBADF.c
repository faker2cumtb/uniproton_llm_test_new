
/* 

【测试目的】:
文件描述符无效时，测试lseek是否返回-1,错误码EBADF。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
设置设备描述符为-1；文件描述符无效，设置文件偏移位置，是否返回-1。

【预期结果】:
文件描述符无效，设置文件偏移位置，返回-1；错误码为EBADF。

【评价准则】:
与预期的测试结果一致

*/

/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Lseek_EBADF( )
{
    unsigned int fd = -1 ;
    unsigned int ret;
    int failed = 0;
    /*请求驱动控制设备处理例程*/
    ret = lseek(fd, 10, 0);
    if (ret != -1)
    {
        failed = 1;
        TEST_ERRPRINT("lseek");
    }
    if (errno != EBADF)
    {
        failed = 1;
        TEST_FAILPRINT("errno error");
    }
	
    if (!failed)
    {
        TEST_OKPRINT();
        return	PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("test fail");
        return	PTS_FAIL;
    }


}
