
/*
【测试目的】:
文件描述符无效时，测试调用fseek，是否返回错误码EBADF 。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建无效的文件描述符，调用fread判断其返回错误码 。

【预期结果】:
fseek返回错误码EBADF。

【评价准则】:
与预期的测试结果一致

*/

/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>
#include <stdio.h>
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Fseek_006()
{
	FILE * fd ={0} ;
    int ret=0;


    ret=fseek(fd,0,NULL);
    if((ret==EOF) && (errno ==EBADF))
    {
    	TEST_OKPRINT();

    }
    else
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
	}

    fclose(fd);
}
