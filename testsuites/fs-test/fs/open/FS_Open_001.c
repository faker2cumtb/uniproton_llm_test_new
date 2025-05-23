
/*

【测试目的】:
当输入参数有效,测试open打开文件是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
输入正常的参数；查看接口返回值。

【预期结果】:
返回值为0.

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
int	OS_FS_Open_001()
{

    int fp;
    int isOK = 0;

    char path[50] = FS_ROOT;
    strcat(path, "/text.txt");
    
    /*test*/
    fp = open(path, O_RDWR | O_CREAT, 0777);
    if (fp == -1)
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
    
    close(fp);
    remove(path);

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
