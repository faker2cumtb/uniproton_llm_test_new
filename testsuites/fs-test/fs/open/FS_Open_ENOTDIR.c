
/* 

【测试目的】:
文件路径中某个目录项不是目录,测试open打开文件是否成功，是否返回的错误号为ENOTDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
打开的文件时，传入路径中目录项不是目录；查看接口返回值。

【预期结果】:
返回值为-1。错误号为ENOTDIR。

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
int	OS_FS_Open_ENOTDIR()
{
    int fp;
    int isOK = 0;    
 	char path[50] = FS_ROOT;
 	char path2[50] = FS_ROOT;
    strcat(path, "/text.txt");
	errno = 0;
    fp = open(path, O_CREAT, 0777);
    if (fp == -1)
    {
        TEST_ERRPRINT("fopen");
    }
    
    close(fp);
    strcat(path2, "/text.txt/text.txt");
    fp = open(path2, O_RDONLY, 0777);
    printf("errno=%d\n", errno);
    if (fp != -1  || errno != ENOTDIR)//A component of the path prefix is not a directory
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
    
    remove(path);
    if (isOK)
    {
        TEST_ERRPRINT("fopen");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
