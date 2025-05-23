
/* 

【测试目的】:
参数 fildes关联的文件不适合执行 read操作时，测试read读操作是否成功，是否返回的错误号为EBADF。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
参数 fildes关联的文件不适合执行 read 操作；查看接口返回值。

【预期结果】:
返回值为-1，错误号为EBADF。

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
int	OS_FS_Read_EBADF2()
{
    int isOK = 0;
    int ret = -1, fdsrc = -1;
    char a[100] = "";
    char path[50] = FS_ROOT;
    strcat(path, "/ST_INTE_FS_READ_002.txt");
    
	remove(path);
	ret = open(path, O_RDWR|O_CREAT, 0777);
    // ret = creat(path, 0777);
    if (-1 == ret)
    {
        TEST_ERRPRINT("test errno");
        isOK = 1;
    }
    close(ret);
    fdsrc = open(path, O_WRONLY, 0777);
    if (-1 == fdsrc)
    {
        TEST_FAILPRINT("test errno");
        isOK = 1;
    }
    ret = write(fdsrc, "abcde", 5);
    if (ret != 5 )
    {

        TEST_ERRPRINT("read");
    }
    ret = read(fdsrc, a, 4);
    printf("ret=%d,errno=%d\n", ret, errno);
    if (ret != -1 || errno != EBADF) //The fildes argument is not a valid file descriptor open for reading
    {
    	TSTDEF_FAILPRINT(errno);
        isOK = 1;
    }
	
	close(fdsrc);
	
	remove(path);
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
