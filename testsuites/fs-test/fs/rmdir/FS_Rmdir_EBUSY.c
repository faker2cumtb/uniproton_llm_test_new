
/* 
【测试目的】:
当输入参数path为当前的工作目录时，测试rmdir删除文件夹是否成功，是否返回的错误号为EBUSY。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用chdir命令切换当前目录；调用rmdir删除当前目录；判断返回值及错误码。

【预期结果】:
返回值为-1,错误号为EBUSY。

【评价准则】:
与预期的测试结果一致
 */

/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"
#include <dirent.h>
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
int OS_FS_Rmdir_EBUSY()
{
    int	 iRet;
    char rmdirfile[50] = FS_ROOT;

    strcat(rmdirfile, "/RmdirEBUSY");
    
    errno = 0;
    /*test*/
    mkdir(rmdirfile, 0777);

    //切换目录
    iRet = chdir(rmdirfile);
    printf("iRet =%d errno=%d\n", iRet, errno);
    iRet = rmdir(rmdirfile);
    printf("iRet =%d errno=%d\n", iRet, errno);
    if ((0 != iRet) || (EBUSY != errno))
    {
        TSTDEF_ERRPRINT(errno);
    	rmdir(rmdirfile);
        return PTS_FAIL;
    }

    else
    {
        chdir(FS_ROOT);
    	rmdir(rmdirfile);
        TEST_OKPRINT();
        return PTS_PASS;
    }

}
