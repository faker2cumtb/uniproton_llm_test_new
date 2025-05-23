
/* 

【测试目的】:
当输入参数路径path长度超过PATH_MAX或path中某个目录项名称长度超过时，测试rmdir删除文件夹是否成功，是否返回的错误号为ENAMETOOLONG。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用rmdir命令删除path中某个目录项名称长度超过NAME_MAX的目录；判断返回值及错误码。

【预期结果】:
返回值为-1,错误号为ENAMETOOLONG。

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
/****************************** 实现部分 *********************************/
/**
 * @brief
 *  初始化任务。
 *
 * @param[in] 无。
 *
 * @return	 PTS_PASS测试成功，PTS_FAIL测试失败。
 */
int OS_FS_Rmdir_ENAMETOOLONG()
{
    int failed = 0,i=0;
    int	 iRet;
    char rmname[PATH_MAX+10]="/test";
    /*test*/

    for(i=0;i<PATH_MAX;i++)
    {
    	strcat((char *)rmname, "1");
    }
//    iRet = rmdir("/yaffs2/text11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111");
    iRet = rmdir((const char *)rmname);
    if ((-1 != iRet) && (ENAMETOOLONG != errno))
    {
        TEST_ERRPRINT("TEST FAIL");
        failed = 1;
    }

    if (!failed)

    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("TEST FAIL");
        return PTS_FAIL;
    }
}
