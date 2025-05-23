
/*

【测试目的】:
当输入参数文件名长度大于255，文件某个目录长度大于255时，测试truncate截断文件是否成功，是否返回的错误码为ENAMETOOLONG。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
文件名长度大于255，调用truncate截断文件。文件某个目录长度大于255，调用truncate截断文件。判断返回值。

【预期结果】:
返回值为-1；错误码为ENAMETOOLONG。

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
int OS_FS_Truncate_ENAMETOOLONG()
{
    int flag = 0,i=0;
    int ret = 0;
    char trname[PATH_MAX+20]=FS_ROOT;
    char drname[PATH_MAX+20]=FS_ROOT;
    /*截取文件，文件名超过255*/
    for(i=0;i<PATH_MAX;i++)
    {
    	strcat(trname, "1");
    }
    strcat(trname, ".txt");
    ret = truncate((const char *)trname,0);
    if (ret != -1 || errno != ENAMETOOLONG)
    {
        TEST_FAILPRINT("truncate截取文件失败");
        flag = 1;
    }

    /*截取文件，文件某个目录名超过255*/
    for(i=0;i<PATH_MAX;i++)
    {
    	strcat(drname, "1");
    }
   	strcat(drname, "/a.txt");
   	ret = truncate((const char *)drname, 0);
   	if (ret != -1 || errno != ENAMETOOLONG)
    {
        TEST_FAILPRINT("truncate截取文件失败");
        flag = 1;
    }
    /*判断测试结果*/
    if (flag == 1)
    {
        TEST_FAILPRINT("truncate");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
