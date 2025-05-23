
/* 

【测试目的】:
当输入参数都有效时，测试read读操作是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
输入有效的参数，调用读接口；查看接口返回值。

【预期结果】:
返回值大于0。

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
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Read_001()
{

    int isOK = 0;
    int ret, fdsrc,fd;
    char wrbuf[51] = "test write data";
    char a[100] = "\0";
    char path[50] = FS_ROOT;
    strcat(path, "/OS_FS_Read_001.txt");
    fd = open(path,   O_RDWR|O_CREAT, 0777);
    if (-1 == fd)
    {
        TEST_ERRPRINT("test errno");
        isOK = 1;
    }
    ret = write(fd, wrbuf, 10);
    if (10 != ret)
    {
        TSTDEF_ERRPRINT(errno);
    }
    ret = read(fd, a, 4);
    if (ret == -1)
    {
    	TSTDEF_ERRPRINT(errno);
    	close(fd);
    	remove(path);
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
    	close(fd);
    	remove(path);
        return PTS_PASS;
    }
}
