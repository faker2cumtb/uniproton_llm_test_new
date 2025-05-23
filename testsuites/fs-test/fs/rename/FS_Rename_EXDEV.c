
/* 
【测试目的】:
当参数 src或 dest指向的路径不属于某个文件系统或属于不同的文件系统时，测试rename重命名是否成功，是否返回的错误号为EXDEV。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
参数 src或 dest指向的路径不属于某个文件系统或属于不同的文件系统，调用rename接口；查看接口返回值。

【预期结果】:
失败返回－1，错误号为EXDEV。

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
int	OS_FS_Rename_EXDEV()
{

    int isOK = 0;
    int ret;
    char path[250] = FS_ROOT;
    strcat(path, "/ST_INTE_FS_RENAME_005.txt");

    ret = rename("/COM1", path);
    printf("ret=%d,errno=%d\n", ret, errno);
    if (ret != -1 || errno != EXDEV)
    {

        TSTDEF_ERRPRINT(errno);
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
