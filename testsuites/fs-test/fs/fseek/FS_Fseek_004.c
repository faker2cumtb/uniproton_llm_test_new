
/* 

【测试目的】:
文件描述符有效时，测试调用fseek,模式选择SEEK_END或SEEK_CUR，是否能重定位指定文件的读写位置量 。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的fd，调用fwrite写入10个字节，调用fseek，模式选择SEEK_END能否重定位指定文件的读写位置量；调用fseek传参 SEEK_CUR能否重定位指定文件的读写位置量。

【预期结果】:
fseek能重定位指定文件的读写位置量。

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
int	OS_FS_Fseek_004()
{
	FILE * fd ={0} ;
    int ret=0,ret1=0;
    int failed = 0;
    char filename[100] =FS_ROOT;
    char wbuf[20]="0123456789";
    char rbuf[20]={0};

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

    ret=fseek(fd,-(ftell(fd)),SEEK_END);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }
    ret1=ftell(fd);
    if(ret1!=0)
    {
    	printf("ftell failed ,ret1 is %d errno is %d\n",ret1,errno);
        TEST_FAILRINT();
    }

    ret=fseek(fd,5,SEEK_CUR);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }
    ret1=ftell(fd);
    if(ret1!=5)
    {
    	printf("ftell failed ,ret1 is %d errno is %d\n",ret1,errno);
        TEST_FAILRINT();
    }

    ret=fread((void *)rbuf,1 , 2, fd);
    if(ret==0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    printf("rbuf[0] is %c rbuf[1] is %c  \n",rbuf[0],rbuf[1]);
    if(strcmp(rbuf, "56")==0)
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
