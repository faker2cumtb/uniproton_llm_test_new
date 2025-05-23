
/* 

【测试目的】:
目录下存在文件时，测试rmdir删除文件夹是否成功，是否返回的错误码为ENOTEMPTY。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建目录"rmdirdir"；创建文件"/rmdirdir/rmdirfile.txt"；调用rmdir命令删除目录"/rmdirdir"；判断返回值。

【预期结果】:
返回值为-1,错误码为ENOTEMPTY。

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
int OS_FS_Rmdir_ENOTEMPTY()
{
    int	 iRet;
    int fp;
    char rmdir1[50] = FS_ROOT;
    char rmdirfile[50] = FS_ROOT;

    strcat(rmdir1, "/rmdirdir");
    strcat(rmdirfile, "/rmdirdir/rmdirfile.txt");
    
    rmdir(rmdir1);
    iRet = mkdir(rmdir1, 0777);
    if (-1 == iRet)
    {
        TEST_FAILPRINT("test errno");
        failed = 1;
        return PTS_FAIL;
    }
    fp = open(rmdirfile, O_CREAT|O_RDWR, 0777);
    if (-1 == fp)
    {
    	TSTDEF_FAILPRINT(errno);
        failed = 1;
        return PTS_FAIL;
    }
    close(fp);

    iRet = rmdir(rmdir1);
    if( (-1 == iRet)&&(errno == ENOTEMPTY))
    {
        TEST_OKPRINT();
    }
    else
    {
    	TSTDEF_FAILPRINT(errno);
        failed = 1;
    }

    remove(rmdirfile);
    rmdir(rmdir1);

    if (!failed)
    {
        return PTS_PASS;
    }
    else
    {
        return PTS_FAIL;
    }
}
