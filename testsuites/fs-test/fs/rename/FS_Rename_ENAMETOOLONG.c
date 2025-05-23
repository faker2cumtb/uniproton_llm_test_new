
/* 

【测试目的】:
参数路径path长度超过PATH_MAX或path中某个目录项名称长度超过时，测试rename重命名是否成功，是否返回的错误号为ENAMETOOLONG。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
参数路径path长度超过PATH_MAX ，调用rename接口；查看接口返回值。

【预期结果】:
失败返回－1，错误号为ENAMETOOLONG。

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
int	OS_FS_Rename_ENAMETOOLONG( )
{

    int isOK = 0;
    int ret,i=0;
    char Tname[PATH_MAX+10]="/test";

    for(i=0;i<PATH_MAX;i++)
    {
        strcat((char *)Tname, "1");
    }

  ret = rename((const char *)Tname,"/test.txt");
    if (ret != -1 || errno != ENAMETOOLONG)
    {
        TEST_ERRPRINT("read");
        isOK = 1;
    }

    if (isOK)
    {
        TEST_ERRPRINT("open");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
