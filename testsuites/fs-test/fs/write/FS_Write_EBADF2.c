
/*

【测试目的】:
文件描述符对应的文件不支持 write 操作时，测试write写操作是否成功，是否返回的错误码为EBADF。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
文件描述符对应的文件不支持 write 操作时，写文件；判断返回值。

【预期结果】:
返回值-1，错误码为EBADF。

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
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int OS_FS_Write_EBADF2()
{
    int iRet,  ret1;
    int failed = 0;
    char wrbuf[51] = "test write data";
    char filename[50] = FS_ROOT;

    strcat(filename, "/test.txt");

    iRet = open(filename, O_CREAT|O_RDWR, 0777);
    close(iRet);
    ret1 = open(filename, O_RDONLY, 0777);
    if (ret1 == -1)
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
    }
    iRet = write(ret1, wrbuf, 10);
    if (-1 == iRet && errno == EBADF )
    {
        TSTDEF_OKPRINT();
    }
    else
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
    }
    close(ret1);
    /*clear environment*/
    iRet = remove(filename);
    if (!failed)
    {
        return PTS_PASS;
    }
    else
    {
        return PTS_FAIL;
    }
}
