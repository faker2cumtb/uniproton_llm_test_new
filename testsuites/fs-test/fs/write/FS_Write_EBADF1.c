
/* 
【测试目的】:
当文件描述符filedes无效时，测试write写操作是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建文件；写文件；判断返回值。

【预期结果】:
返回值为非0。

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
int OS_FS_Write_EBADF1()
{
    int iRet;
    int failed = 0;
    char wrbuf[51] = "0123456789";
    char filename[50] = FS_ROOT;

    strcat(filename, "/test.txt");

    iRet = write(-1, wrbuf, 10);
    printf("ret =%d errno=%d\n", iRet, errno);
    if (-1 == iRet && errno == EBADF)//The fildes argument is not a valid file descriptor open for writing.
    {
        TSTDEF_OKPRINT();
    }
    else
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
    }
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
