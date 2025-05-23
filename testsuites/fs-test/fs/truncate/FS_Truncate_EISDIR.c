
/* 
【测试目的】:
路径 path指向的文件是一个目录而非常规文件时，测试truncate截断文件是否成功，是否返回的错误码为EISDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用truncate截断文件,文件路径中某个目录项不是目录；判断返回值。

【预期结果】:
返回值为-1；错误码为EISDIR。

【评价准则】:
与预期的测试结果一致

 */

/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"

int OS_FS_Truncate_EISDIR()
{
    int flag = 0;
    int ret = 0;
    char rmdirfile[50] = FS_ROOT;

    strcat(rmdirfile, "/testdir");
    rmdir(rmdirfile);
    mkdir(rmdirfile, 0777);
    ret = truncate(rmdirfile, 0);
    if (ret != -1 || errno != EISDIR)
    {
        TSTDEF_ERRPRINT(errno);
        flag = 1;
    }
	
	rmdir(rmdirfile);
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
