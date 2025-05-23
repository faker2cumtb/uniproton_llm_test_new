
/* 
【测试目的】:
调用close关闭指定文件，如果关闭文件描述符<fd>指定的文件成功，应返回0。

【测试类型】:
功能性测试

【先决条件】:
1.文件系统正常运行

【测试步骤】:
1.调用open创建文件/vnfs/test.txt，如果操作失败，则直接返回测试失败；2.调用close关闭/vnfs/test.txt文件，将返回值赋值给ret；3.判断返回值ret为0，则测试成功，返回测试成功标志，否则测试失败，返回测试失败标志。

【预期结果】:
返回PASS

*/


/****************************** 实现部分 *********************************/
#include <stdio.h>
#include <test.h>
#include "fcntl.h"

int OS_FS_Close_01()
{
	int fd;
	int ret;
	char filename[100] =FS_ROOT;
	strcat(filename, "/test.txt");
	remove(filename);
	/*打开/vnfs/test.txt文件，如果此文件不存在，则创建此文件*/
	fd = open(filename,O_CREAT);
	/*判断是否打开文件成功*/
	if(fd == -1)
	{
        remove(filename);//
        TEST_FAILRINT();
	    c_Results.isPass=PTS_FAIL;
		return PTS_FAIL;
		
	}
	
	/*关闭文件*/
	ret = close(fd);		
	if(ret == 0)
	{
        remove(filename);//
        TEST_OKPRINT();
	    c_Results.isPass=PTS_PASS;
		return PTS_PASS;
	}
	
	else
	{ 
        TEST_FAILRINT();
	    c_Results.isPass=PTS_FAIL;
		return PTS_FAIL;
	}
}
