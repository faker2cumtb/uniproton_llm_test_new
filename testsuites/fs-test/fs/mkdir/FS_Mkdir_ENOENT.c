
/* 
【测试目的】:
以空目录名创建目录，测试mkdir否返回的错误号为ENOENT。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用mkdir命令创建目录""；判断返回值及错误号。

【预期结果】:
返回值为-1，错误号为ENOENT。

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
int OS_FS_Mkdir_ENOENT()
{
    int	 iRet;
    /*test*/
    iRet = mkdir(NULL, 0777);
    if ((-1 == iRet) && (ENOENT == errno)) //满足7714标准，修改错误码为ENOENT 要打开的文件不存在或文件路径不存在，或文件路径为空
    {
        TEST_OKPRINT();
        failed = 0;
    }
    else
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
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
