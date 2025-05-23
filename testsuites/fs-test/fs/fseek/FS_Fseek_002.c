
/* 

【测试目的】:
文件描述符有效时，测试调用fseek,无效的whence参数，是否返回EINVAL 。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的fd，测试调用fseek,无效的whence参数，是否返回EINVAL 。

【预期结果】:
fseek返回错误码EINVAL。

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
int	OS_FS_Fseek_002()
{
	FILE * fd ={0} ;
    int ret=0,flag=0;
    char filename[100] =FS_ROOT;
    char wbuf[20]="0123456789";

    strcat(filename, "/fleektest.txt");
    remove(filename);
    fd=fopen(filename,"w+");
    if(fd ==NULL)
    {
    	TSTDEF_ERRPRINT(errno);
    	TEST_FAILRINT();
    	return -1;
    }

    ret=fwrite((const void *)wbuf, strlen((const void *)wbuf), 1, fd);
    printf("fwrite card is %s\n",filename);
    if(ret==0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    errno=0;
    ret=fseek(fd,0,-1);
    if(ret!=EOF || errno !=EINVAL)
    {
    	printf("fseek test  failed , ret is %d errno is %d\n",ret,errno);
    	TEST_FAILRINT();
        flag=-1;
    }

    ret=fseek(fd,-100,SEEK_SET);
    if(ret!=EOF || errno !=EINVAL)
    {
    	printf("fseek test  failed ,ret is %d  errno is %d\n",ret,errno);
    	TEST_FAILRINT();
        flag=-1;
    }

    if(flag==0)
    {
     	TEST_OKPRINT();
    }
    else
    {
        TEST_FAILRINT();
	}


    fclose(fd);
    remove(filename);
}
