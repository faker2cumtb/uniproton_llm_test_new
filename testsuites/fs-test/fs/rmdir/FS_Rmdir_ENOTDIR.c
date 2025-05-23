
/* 
【测试目的】:
当文件路径中某个目录项不是目录时，测试rmdir删除文件夹是否成功，是否返回的错误号为ENOTDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用rmdir命令文件路径中某个目录项不是目录；判断返回值及错误码。

【预期结果】:
返回值为-1,错误号为ENOTDIR。

【评价准则】:
与预期的测试结果一致

*/

/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"

int OS_FS_Rmdir_ENOTDIR()
{
    int	 iRet;
    int failed = 0;
    char rmdirfile[50] = FS_ROOT;

    strcat(rmdirfile, "/ST_INTE_FS_RMDIR_009.txt");
    
    /*test*/
    iRet=open(rmdirfile,O_RDWR |O_CREAT);
//    iRet = open(rmdirfile, O_CREAT|O_RDWR, 0777);
    if (-1 == iRet)
    {
        TEST_ERRPRINT("TEST FAIL");
        failed = 1;
    }
    iRet = rmdir(rmdirfile);
    if ((-1 != iRet) && (ENOTDIR != errno))
    {
        TEST_ERRPRINT("TEST FAIL");
        failed = 1;
    }
    unlink(rmdirfile);


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
