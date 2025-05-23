
/* 
【测试目的】:
参数 oflag无效,测试open打开文件是否成功，错误号是否为EINVAL。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
打开的文件时，传入的参数 oflag 无效；查看接口返回值。

【预期结果】:
返回值为-1，错误号为EINVAL。

【评价准则】:
与预期的测试结果一致

 */


/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Open_EINVAL()
{
    int fp;
    int isOK = 0;
 	char path[50] = FS_ROOT;
    strcat(path, "/1/1/text.txt");
    
    /*test 错误的oflag*/
    fp = open(path, -1, -1);
    if (fp != -1 || errno != EINVAL)
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
    if (fp != -1)
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
    if (isOK)
    {
        TEST_ERRPRINT("fopen");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
