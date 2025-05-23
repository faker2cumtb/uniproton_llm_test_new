
/* 

【测试目的】:
打开文件时，文件路径的长度超过255个字符,测试open打开文件是否成功，是否返回的错误号为ENAMETOOLONG。。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
打开的文件的文件路径的长度超过260个字符；查看接口返回值。

【预期结果】:
返返回值为-1，错误号为ENAMETOOLONG。

【评价准则】:
与预期的测试结果一致

 */


/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"

int	OS_FS_Open_ENAMETOOLONG()
{
    int fp;
    int isOK = 0,i=0,a=0;
    char tfile[PATH_MAX+10]="/tes";

    /*test 长度257*/
    for (i=0; i < PATH_MAX; ++i)
    {
    	strcat(tfile, "1");
	}
    strcat(tfile, ".txt");
   // printf("tfile is %s\n",tfile);
    fp = open(tfile, O_WRONLY | O_RDWR, 0777);
    if (fp != -1 || errno != ENAMETOOLONG)
    {
    	TEST_FAILPRINT("fopen");
        isOK = 1;
    }
    close(fp);
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
