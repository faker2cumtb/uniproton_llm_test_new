
/* 

【测试目的】:
ENAMETOOLONG 路径 path 长度超过 PATH_MAX 或 path 中某个目录项名称长度超过时，测试stat获取指定文件的信息，是否返回的错误码为ENAMETOOLONG。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
获取的文件路径名称大于PATH_MAX时，调用接口；判断返回值。

【预期结果】:
返回值为-1，错误码为ENAMETOOLONG.

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
int OS_FS_Stat_ENAMETOOLONG()
{
    int failed = 0,i=0;

    int	  ret;
    struct stat st1;
    char stname[PATH_MAX+10]="/yaffs2";
    /*test*/
    for(i=0;i<PATH_MAX;i++)
    {
    	strcat((char *)stname, "1");
    }
   ret = stat((const char *)stname,&st1);
    if (-1 != ret || errno != ENAMETOOLONG)
    {
        TSTDEF_ERRPRINT(errno);
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
