/*
【测试目的】:
文件描述符有效时，测试调用fseek，是否能重定位指定文件的读写位置量 。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的fd，调用fread读出当前fd指向的地址2个字节，再调用fseek偏移3个字节，再调用fread读出当前fd指向地址的2个字节，前后读出的字节是不一样的 。

【预期结果】:
fseek能重定位指定文件的读写位置量。

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
int	OS_FS_Fseek_001()
{
	FILE * fd ={0} ;
    int ret=0,ret1=0;
    int failed = 0;
    char filename[100] =FS_ROOT;
    char wbuf[20]="0123456789";
    char rbuf[20]={0};
    char rbuf1[20]={0};
    strcat(filename, "/fleektest.txt");
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

    ret=fseek(fd,0,NULL);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    ret=fseek(fd,0,SEEK_SET);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    ret=fread((void *)rbuf,1 , 2, fd);
    if(ret==0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    ret=fseek(fd,3,SEEK_SET);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }
    ret1=ftell(fd);
    if(ret1==-1)
    {
    	printf("ftell failed ,errno is %d\n",errno);
        TEST_FAILRINT();
    }
    ret=fread((void *)rbuf1,1 , 2, fd);
    if(ret==0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    printf("rbuf[0] is %c rbuf[1] is %c  rbuf1[0] is %c rbuf1[1]is %c \n",rbuf[0],rbuf[1],rbuf1[0],rbuf1[1]);
    if((strcmp(rbuf, rbuf1)!=0) && (strcmp(rbuf1, "34")==0)&&(ret1==3))
    {
    	TEST_OKPRINT();
    }
    fclose(fd);
    remove(filename);
}
