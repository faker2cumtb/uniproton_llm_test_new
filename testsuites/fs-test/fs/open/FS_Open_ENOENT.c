
/* 
【测试目的】:
要文件路径不存在且设置O_CREAT并包含不存在的目录,要文件路径不存在且未设置O_CREAT，或文件不存在，或文件路径为空，测试open打开文件是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
打开的文件不存在且未设置O_CREAT，或文件路径不存在，或文件路径为空；查看接口返回值和错误码。

【预期结果】:
返回值为-1。错误号为ENOENT。

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
int	OS_FS_Open_ENOENT()
{
    int fp;
    int isOK = 0;
 	char path[50] = FS_ROOT;
 	char p[50] = FS_ROOT;
 	char p1[50] = FS_ROOT;
    strcat(path, "/1/1/text.txt");
    strcat(p, "/1");
    strcat(p1, "/1/1");
    
    remove(path);
    rmdir(p);
    rmdir(p1);
    /*test*/
    fp = open(path, O_CREAT|O_RDWR, 0777);
    if (fp != -1 || errno != ENOENT)
    {
    	printf("errno %d\n",errno);
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
	fp = open(path, 0x0000, 0777);
    if (fp != -1 || errno != ENOENT)
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
    memset(path, 0, sizeof(path));
    strcpy(path, FS_ROOT);
    strcat(path, "text10000000.txt");
    remove(path);
    fp = open(path, 0x0000, 0777);
    if (fp != -1 || errno != ENOENT)
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
    fp = open("", 0x0000, 0777);
    if (fp != -1 || errno != EINVAL)
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
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
