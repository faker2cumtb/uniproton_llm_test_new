/*

【测试目的】:
当pathname指向的目录不存在时，测试chdir改变目录是否成功，错误号是否为ENOTDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用接口remove删除ST_INTE_FS_CHDIR_003.txt文件；以ST_INTE_FS_CHDIR_003.txt作为当前工作目录参数调用接口chdir改变当前工作目录；调用接口remove删除ST_INTE_FS_CHDIR_003.txt文件；判断返回值。

【预期结果】:
返回值等于-1。 错误号为ENOTDIR。

【评价准则】:
与预期的测试结果一致

*/

#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>

int OS_FS_Chdir_ENOTDIR()
{
    int ret,fd;
    int flag = 0;
    char path[50] = FS_ROOT;

    strcat(path, "/ST_INTE_FS_CHDIR_003.txt");

    remove(path);
    fd = open(path, O_RDWR|O_CREAT, 0777);
    if (fd < 0)
    {
        TEST_FAILPRINT("creat is fail");
        flag = 1;
    }
    close(fd);
    ret = chdir(path);
    if ( ret != -1 || errno != ENOTDIR)
    {
    	TSTDEF_FAILPRINT(errno);
        flag = 1;
    }
    remove(path);
    if (flag == 1)
    {
    	TSTDEF_FAILPRINT(errno);
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
