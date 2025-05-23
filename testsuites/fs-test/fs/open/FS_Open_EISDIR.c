
/* 

【测试目的】:
打开的文件为目录，且设置了O_WRONLY 或O_RDWR,测试open打开文件是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
打开的文件为目录，且设置了O_WRONLY 或O_RDWR；查看接口返回值。

【预期结果】:
返回值为-1， 错误号为EISDIR。

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
int	OS_FS_Open_EISDIR()
{
    int isOK = 0;
    int fp;
    char path[50] = FS_ROOT;
    strcat(path, "/test001");
    
    /*test*/
    fp = mkdir(path, 0000);
    if (fp == -1)
    {
        TEST_ERRPRINT("mkdir");
    }
    //fp = open(path, O_WRONLY | O_RDWR);
    fp = open(path,  O_RDWR);
    printf("fp=%d,errno=%d\n", fp, errno);
    if (fp != -1 || errno != EISDIR)
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
        printf("errno == %d\n",errno);
    }
    rmdir(path);
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
