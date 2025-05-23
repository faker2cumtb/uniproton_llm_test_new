
/* 
【测试目的】:
文件描述符无效时，测试调用ftell，是否返回错误码EBADF 。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建无效的fd，调用ftell返回错误码EBADF 。

【预期结果】:
ftell返回错误码EBADF。

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
int	OS_FS_Ftell_002()
{
    int ret=0;
    int flag = 0;
    // FILE nullFp = {0};
    FILE *nullFp = NULL;


    ret=ftell(NULL);
    if(ret!=-1 ||errno !=EBADF)
    {
    	flag=-1;
    	printf("ftell failed ret is %d  errno is %d\n",ret,errno);
    }


    ret=ftell(nullFp);
    if(ret!=-1 ||errno !=EBADF)
    {
    	flag=-1;
    	printf("ftell failed ret is %d  errno is %d\n",ret,errno);
    }

    if(flag==0)
    {
    	TEST_OKPRINT();

    }
    else
    {
        TEST_FAILRINT();
	}

}
