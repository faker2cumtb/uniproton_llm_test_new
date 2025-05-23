/****************************************************************************
 *				
 *
 * 		 Copyright (C) 2000-2020 CoreTek Systems Inc. All Rights Reserved.
 ***************************************************************************/

/* 

【测试目的】:
当超出文件范围的偏移量,fseek返回错误码EOVERFLOW。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的fd，调用fseek以当前位置向前超出文件范围的偏移量，判断其返回值。

【预期结果】:
fseek返回EOF，并置错误码为EOVERFLOW。

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
int	OS_FS_Fseek_008()
{
	FILE * fd ={0} ;
    int ret=0;
    char filename[100] =FS_ROOT;
    char wbuf[20]="0123456789";
    char rbuf[20]={0};
    char rbuf1[20]={0};
    long long offset = 9223372036854775807;
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

    ret=fseek(fd,offset,SEEK_END);
    if((ret!=EOF)||(errno !=EOVERFLOW)) //posix标准：For fseek(), the resulting file offset would be a value which cannot be represented correctly in an object of type long.
    {
    	TSTDEF_ERRPRINT(errno);
        TEST_FAILRINT();
        fclose(fd);
        remove(filename);
        return -1;
    }
    else{
    	TEST_OKPRINT();
        fclose(fd);
        remove(filename);
        return 0;
    }

}
