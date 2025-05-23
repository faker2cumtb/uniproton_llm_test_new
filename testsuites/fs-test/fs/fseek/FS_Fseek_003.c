
/* 
【测试目的】:
文件描述符为socket时，测试fseek是否返回-1,错误码ESPIPE;文件描述符有效且不支持_seek操作时，测试调用fseek,是否返回ESPIPE 。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的fd，设置fd_seek为null，测试调用fseek,是否返回ESPIPE 。

【预期结果】:
fseek返回错误码ESPIPE。

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
int	OS_FS_Fseek_003()
{
	FILE * fd ={0} ;
    int ret=0;
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
    if(ret==0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    // // fd->_seek=NULL;

    errno=0;
    ret=fseek(fd,0,SEEK_SET);
    if(ret==EOF && errno ==ESPIPE)
    {
    	TEST_OKPRINT();

    }
    else
    {
    	printf("fseek test  failed ,errno is %d\n",errno);
    	TEST_FAILRINT();
	}

    fclose(fd);
    remove(filename);
}
