
/* 

【测试目的】:
当要打开的文件不存在且未设置 O_CREAT，或文件路径不存在，或文件路径为空时， 测试truncate截断文件是否成功，是否返回的错误码为ENOENT。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
当要打开的文件不存在且未设置 O_CREAT，或文件路径不存在，或文件路径为空时，测试truncate截断文件是否成功；判断返回值。

【预期结果】:
返回值为-1；错误码为ENOENT。

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
int OS_FS_Truncate_ENOENT()
{
    int flag = 0;
    int ret = 0;
    char rmdirfile[80] = FS_ROOT;

    strcat(rmdirfile, "/unknown/ST_FUNC_FS_TRUNCATE_004.txt");
    
    ret = truncate(rmdirfile, 0);
    if (ret != -1 || errno != ENOENT)
    {
        TEST_FAILPRINT("truncate");
        flag = 1;
    }

    ret = truncate("", 0);
    if (ret != -1 || errno != ENOENT)
    {
        TSTDEF_ERRPRINT(errno);
        flag = 1;
    }

    ret = truncate(NULL, 0);
    if (ret != -1 || errno != ENOENT)
    {
        TEST_FAILPRINT("truncate截取文件失败");
        flag = 1;
    }
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
