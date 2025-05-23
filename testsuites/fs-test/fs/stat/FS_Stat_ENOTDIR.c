
/*

【测试目的】:
当文件路径中某个目录项不是目录时，测试stat获取指定文件的信息，是否返回的错误码为ENOTDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建一个文件text.txt；在以/text.txt/b为路劲调用接口；判断返回值。

【预期结果】:
返回值为-1。错误码为ENOTDIR.

【评价准则】:
与预期的测试结果一致

*/

/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"
#include <sys/stat.h>
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
int OS_FS_Stat_ENOTDIR()
{
    int failed = 0;

    int	 iRet, ret;
    struct stat st1;
    char rmdirfile[50] = FS_ROOT;

    strcat(rmdirfile, "/text.txt");
    
    remove(rmdirfile);
    iRet = open(rmdirfile,  O_CREAT, 0777);
//    iRet = open(rmdirfile, O_CREAT|O_RDWR, 0777);
    if (-1 == iRet)
    {
        TEST_ERRPRINT("test errno");
        failed = 1;
    }
    strcat(rmdirfile, "/b");
    ret = stat(rmdirfile, &st1); //目录项不是目录
    printf("ret=%d,errno=%d\n", ret, errno);
    if (-1 != ret || errno != ENOTDIR)//7714要求ENOTDIR 文件路径中某个目录项不是目录
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
    }
    close(iRet);
    remove(rmdirfile);

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
