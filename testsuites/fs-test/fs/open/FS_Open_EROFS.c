
/* 
【测试目的】:
文件系统为只读类型，但设置了 O_WRONLY、O_RDWR、O_CREAT 或 O_TRUNC,测试open打开文件是否成功,返回的错误号是否为EROFS。。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
文件系统为只读类型；oflag打开选项,设置了 O_WRONLY、O_RDWR、O_CREAT 或 O_TRUNC；查看接口返回值。

【预期结果】:
返回值为-1,错误号为EROFS。

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
int	OS_FS_Open_EROFS()
{

    int fp;
    int isOK = 0;

    /*test*/
    char path[50] = FS_ROOT;
    strcat(path, "/EROFS.txt");

    fp = open(path, O_RDWR, 0777);
    printf("fd=%d,errno=%d", fp, errno);
    if (fp != -1 || errno != EROFS)
    {
    	TSTDEF_FAILPRINT(errno);
        isOK = 1;
    }

    fp = open(path, O_WRONLY | O_TRUNC, 0777);
    printf("fd=%d,errno=%d", fp, errno);
    if (fp != -1 || errno != EROFS)
    {
    	TSTDEF_FAILPRINT(errno);
        isOK = 1;
    }



    fp = open(path, O_RDWR | O_TRUNC, 0777);
    printf("fd=%d,errno=%d", fp, errno);
    if (fp != -1 || errno != EROFS)
    {
    	TSTDEF_FAILPRINT(errno);
        isOK = 1;
    }

    fp = open(path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    printf("fd=%d,errno=%d", fp, errno);
    if (fp != -1 || errno != EROFS)
    {
    	TSTDEF_FAILPRINT(errno);
        isOK = 1;
    }


    remove(path);
    if (isOK)
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
