
/* 

【测试目的】:
当偏移方向是向文件头方向偏移时，偏移的位置超出文件头,fseek返回错误码EINVAL。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的fd，调用fseek以当前位置向前移动10个偏移量，判断其返回值，以文件末尾端向前偏移20个偏移量，判断其返回值。

【预期结果】:
fseek返回EOF，并置错误码为EINVAL。

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
int	OS_FS_Fseek_007()
{
	FILE * fd ={0} ;
    int ret=0;
    char filename[100] =FS_ROOT;
    char wbuf[20]="0123456789";
    char rbuf[20]={0};
    char rbuf1[20]={0};

    strcat(filename, "/fleektest07.txt");

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

    ret=fseek(fd,-10,SEEK_CUR);
    if((ret!=EOF)||(errno !=EINVAL))
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    ret=fread((void *)rbuf1,1 , 2, fd);
    if(ret==0)
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    ret=fseek(fd,-20,SEEK_END);
    if((ret!=EOF)||(errno !=EINVAL))
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
    }

    if( strcmp(rbuf1, "23")==0)
    {
    	TEST_OKPRINT();
    }
    fclose(fd);
    remove(filename);
}
