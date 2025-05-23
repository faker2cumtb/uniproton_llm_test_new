
/*

【测试目的】:
删除不存在的目录时，测试rmdir删除文件夹是否成功，是否返回的错误码为ENOENT。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用rmdir命令删除不存在的目录"inexistdir"；判断返回值及错误码。

【预期结果】:
返回值为-1,错误码为ENOENT。

【评价准则】:
与预期的测试结果一致

 */

/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"
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
int OS_FS_Rmdir_ENOENT()
{
    int	 iRet;
    char rmdirfile[50] = FS_ROOT;

    strcat(rmdirfile, "/inexistdir");
    
    /*test*/
    iRet = rmdir(rmdirfile);
    if ((-1 == iRet) && (ENOENT == errno))
    {
        TEST_OKPRINT();
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
