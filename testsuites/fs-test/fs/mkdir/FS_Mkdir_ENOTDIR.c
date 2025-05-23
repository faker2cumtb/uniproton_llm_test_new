
/* 
【测试目的】:
在一不存在的目录下创建新目录，测试mkdir创建目录是否成功，错误码是否为错误码为ENOTDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用mkdir命令创建目录"/inexistdir/mkdirdir"；判断返回值。

【预期结果】:
返回值为-1。 错误码为ENOTDIR。

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
int OS_FS_Mkdir_ENOTDIR()
{
    int	 iRet;
    char dirname[50] = FS_ROOT;

    strcat(dirname, "/inexistdir/mkdirdir");

    iRet = mkdir(dirname, 0777);
    if (-1 != iRet)
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
    }
    else
    {
        TEST_OKPRINT();
    }

    if (!failed)
    {
        return PTS_PASS;
    }
    else
    {
        return PTS_FAIL;
    }
}
