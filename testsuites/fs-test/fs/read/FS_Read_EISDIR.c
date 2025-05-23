
/* 

【测试目的】:
指定的参数文件描述符 fildes 为目录，而目录不允许使用 read 操作读取时，测试read读操作是否成功，是否返回的错误号为EISDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
指定的参数文件描述符 fildes 为目录，而目录不允许使用 read 操作读取；查看接口返回值。

【预期结果】:
返回值为-1，错误号为EISDIR。

【评价准则】:
与预期的测试结果一致

 */


/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"
#include <dirent.h>
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Read_EISDIR()
{
    int isOK = 0;
    int ret, fd1, fd2;
    DIR *fd;
    char a[100] = "";
    char path[100] = FS_ROOT;
    strcat(path, "/ST_INTE_FS_READ_003");
    remove(path);
    /*创建目录*/
    fd2 = mkdir(path, O_WRONLY);
    if ((int)fd2 == -1)
    {
    	printf("before opendir\n");
		TEST_ERRPRINT(strerror(errno));
        isOK = 1;
    }
    /*打开目录*/
    printf("before opendir\n");
    fd = opendir(path);
    if ((int)fd == -1)
    {
    	printf(" opendir   failed \n");
		TEST_ERRPRINT(strerror(errno));
        isOK = 1;
    }
    fd1 = path;
    printf("before read\n");
    ret = read(fd1, a, 4);
    printf("ret=%d,errno=%d\n", ret, errno);
    if (ret != -1 || errno != EISDIR )
    {
		TEST_ERRPRINT(strerror(errno));
        isOK = 1;
    }

    if (isOK)
    {
		TEST_ERRPRINT(strerror(errno));
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
