
/* 
【测试目的】:
当系统中某个目录项不是目录时，测试rename重命名是否成功，是否返回的错误号为ENOTDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
系统中打开目录不是目录时，调用rename接口；查看接口返回值。

【预期结果】:
失败返回－1，错误号为ENOTDIR。

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
int	OS_FS_Rename_ENOTDIR()
{

    int isOK = 0;
    int ret;
    char path[250] = FS_ROOT;
    char path1[250] = FS_ROOT;
    strcat(path, "/ST_INTE_FS_RENAME_004.txt");
    strcat(path1, "/ST_INTE_FS_RENAME_004_123");

	ret = open(path,   O_RDWR|O_CREAT, 0777);
    // ret = creat(path, 0777);
    if (-1 == ret)
    {
        TEST_ERRPRINT(strerror(errno));
 //       isOK = 1;
    }
    
    close(ret);

    ret = mkdir(path1, 0777);
    if (-1 == ret)
    {
        TEST_ERRPRINT(strerror(errno));
//        isOK = 1;
    }

    close(ret);

    ret = rename(path1, path);
    printf("ret=%d,errno=%d\n", ret, errno);
    if (ret != -1 || errno != ENOTDIR )//7714错误码为 ENOTDIR
    {

        TEST_ERRPRINT("read");
        isOK = 1;
    }

    remove(path);
    remove(path1);
    if (isOK)
    {
        TEST_ERRPRINT("open");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
