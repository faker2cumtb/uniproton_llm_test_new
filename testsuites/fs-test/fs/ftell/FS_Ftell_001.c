
/*

【测试目的】:
文件描述符有效时，测试调用ftell，能否获取指定文件的当前读写位置 。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的fd，调用fwite向指定文件中写入字符，调用ftell获取当前读写位置；调用fseek偏移0个字节，调用ftell获取当前读写位置；调用fseek偏移4个字节，调用ftell获取当前读写位置 。

【预期结果】:
ftell成功获取指定文件的当前读写位置。

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
int	OS_FS_Ftell_001()
{
	FILE * fd ={0} ;
    int ret=0;
    int failed = 0;
    char filename[100] =FS_ROOT;
    char wbuf[20]="0123456789";
    char rbuf[20]={0};
    char rbuf1[20]={0};
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
    ret=ftell(fd);
    if(ret!=10)
    {
     	printf("ftell failed , ret=%d errno is %d\n",ret,errno);
        TEST_FAILRINT();
    }
    ret=fseek(fd,-1,SEEK_CUR);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }
    ret=ftell(fd);
    if(ret!=9)
    {
     	printf("ftell failed , ret=%d errno is %d\n",ret,errno);
        TEST_FAILRINT();
    }
    ret=fseek(fd,2,SEEK_CUR);
    if(ret!=0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }
    ret=ftell(fd);
    if(ret==11)
    {
    	TEST_OKPRINT();

    }
    else
    {
    	printf("ftell failed , ret=%d errno is %d\n",ret,errno);
        TEST_FAILRINT();
	}

    fclose(fd);
    remove(filename);
}
