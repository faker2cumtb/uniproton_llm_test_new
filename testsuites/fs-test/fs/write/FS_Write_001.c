
/* 
【测试目的】:
当输入参数有效时，测试write写操作是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
参数fd为文件描述符；写文件；判断返回值。

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
int OS_FS_Write_001()
{
    int iRet, ret1;
    char wrbuf[51] = "test write data";
    char filename[50] = FS_ROOT;

    strcat(filename, "/test.txt");
    iRet = open(filename,   O_RDWR|O_CREAT, 0777);
    // iRet = open(filename, O_CREAT|O_RDWR, 0777);;
    close(iRet);
    ret1 = open(filename, O_RDWR, 0777);
    if (ret1 == -1)
    {
        TEST_ERRPRINT("creat");
    }
    iRet = write(ret1, wrbuf, 10);
    if (10 == iRet)
    {
        TSTDEF_OKPRINT();
    }
    else
    {
        TSTDEF_ERRPRINT(errno);

    }
    iRet = close(ret1);
    if (0 == iRet)
    {
        TEST_OKPRINT();
    }
    else
    {
        TSTDEF_ERRPRINT(errno);

    }
    remove(filename);
    return PTS_PASS;

}
