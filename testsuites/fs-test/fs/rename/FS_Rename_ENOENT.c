
/* 

【测试目的】:
当要打开的文件不存在且未设置O_CREAT，或文件路径不存在，或文件路径为空时，测试rename重命名是否成功，是否返回的错误号为ENOENT。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
要打开的文件不存在且未设置O_CREAT，调用rename接口；要打开的文件路径不存在，调用rename接口；要打开的文件路径为空，调用rename接口；查看接口返回值。

【预期结果】:
失败返回－1，错误号为ENOENT。

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
int	OS_FS_Rename_ENOENT()
{

    int isOK = 0;
    int ret;
    char path[250] = FS_ROOT;
    char path1[250] = FS_ROOT;
    strcat(path, "/text11222221.txt");
    strcat(path1, "/text112.txt");
    
    //文件不存在
    ret = rename(path, path1);
    if (ret != -1 || errno != ENOENT)
    {
        TEST_ERRPRINT("rename");
        isOK = 1;
    }
    ret = rename(NULL, path1); //路径为空
    printf("ret=%d,errno=%d\n", ret, errno);
    if (ret != -1 || errno != ENOENT)
    {
        TEST_ERRPRINT("rename");
        isOK = 1;
    }
    strcat(path, "/text11222221");
    strcat(path1, "/text112");
    ret = rename(path, path1); //文件路径不存在
    printf("ret=%d,errno=%d\n", ret, errno);
    if (ret != -1 || errno != ENOENT)
    {
        TEST_ERRPRINT("rename");
        isOK = 1;
    }
    if (isOK)
    {
        TEST_ERRPRINT("rename");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
