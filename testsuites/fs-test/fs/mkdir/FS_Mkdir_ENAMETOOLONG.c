
/* 
【测试目的】:
目录名长度超过最大值，测试mkdir是否返回的错误号为ENAMETOOLONG。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用mkdir命令创建目录名长256；判断返回值及错误号。

【预期结果】:
函数返回-1；错误号为ENAMETOOLONG。

【评价准则】:
与预期的测试结果一致
*/

/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>
/**************************** 定义部分 *****************************************/
static int failed = 0;
/****************************** 实现部分 *********************************/
/**
 * @brief
 *  初始化任务。
 *
 * @param[in] 无。
 *
 * @return	 PTS_PASS测试成功，PTS_FAIL测试失败。
 */
int OS_FS_Mkdir_ENAMETOOLONG()
{
    int	 iRet, i;
    char pathname[280] = FS_ROOT;

    /*test*/
    strcat(pathname, "/");
    for (i = 0; i < PATH_MAX; i++)
    {
        strcat(pathname, "a");
    }

    iRet = mkdir(pathname, 0777);
    if ((-1 == iRet) && (ENAMETOOLONG == errno))
    {
        TEST_OKPRINT();
    }
    else
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
    }

    rmdir(pathname);

    if (!failed)
    {
        return PTS_PASS;
    }
    else
    {
        return PTS_FAIL;
    }
}
