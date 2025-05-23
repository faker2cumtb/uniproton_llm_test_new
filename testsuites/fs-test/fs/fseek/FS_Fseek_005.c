
/* 
【测试目的】:
文件描述符有效时，测试调用fseek重定位指定文件的读写位置成功，当offset偏移量大于文件长度，是否以二进制0的方式进行填充。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的fd，调用fopen创建文件，再调用fseek偏移2个字节，再调用fwrite写入2个字节，fseek将读写位置设置到文件开头，fread读出2个字节判断是否为0。

【预期结果】:
fseek能重定位指定文件的读写位置量，当offset偏移量大于文件长度，以二进制0的方式进行填充。

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
int	OS_FS_Fseek_005()
{
	FILE * fd ={0} ;
    int ret=0;
    int failed = 0;
    char filename[100] =FS_ROOT;
    char wbuf[20]="0123456789";
    char wbuf1[20]="a";
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

    ret=fseek(fd,2,SEEK_SET);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
    	TEST_FAILRINT();
    }

    ret=fwrite((const void *)wbuf, strlen((const void *)wbuf), 1, fd);
    if(ret==0)
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

    if(rbuf[0]!=0 ||rbuf[1]!=0)
    {
    	printf("rbuf[0] is %x ,rbuf[1] is %x\n",rbuf[0],rbuf[1]);
        TEST_FAILRINT();
    }


    fclose(fd);
    fd=fopen(filename,"a+");
    if(fd ==NULL)
    {
    	TSTDEF_ERRPRINT(errno);
    	TEST_FAILRINT();
    	return -1;
    }

    ret=fseek(fd,1,SEEK_CUR);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    ret=fwrite((const void *)wbuf1, strlen((const void *)wbuf1), 1, fd);
    if(ret==0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }
    ret=fseek(fd,-2,SEEK_END);
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

    if(rbuf[0]=='9' && rbuf[1]=='a')
    {
    	TEST_OKPRINT();
    }
    else
    {
    	printf("rbuf[0] is %x ,rbuf[1] is %c\n",rbuf[0],rbuf[1]);
        TEST_FAILRINT();
	}

    fclose(fd);
    remove(filename);
}
