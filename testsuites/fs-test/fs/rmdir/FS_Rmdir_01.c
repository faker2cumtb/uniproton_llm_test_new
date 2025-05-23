
/* 
【测试目的】:
调用rmdir用于在文件系统中删除指定名称的目录，如果在文件系统中成功删除<path>指定的目录，应返回0。

【测试类型】:
功能性测试

【先决条件】:
1.文件系统正常运行

【测试步骤】:
1.调用mkdir创建目录"/vnfs/testdir2"；2.以调用rmdir删除这个目录，并将返回值赋值给iRet；3.判断iRet的值是否为0，若iRet为0，则返回测试成功标志，否则测试失败，返回测试失败标志。

【预期结果】:
返回PASS

【评价准则】:
与预期的测试结果一致

 */


/****************************** 实现部分 *********************************/
#include <stdio.h>
#include <test.h>
#include "fcntl.h"
int OS_FS_Rmdir_01()
{
	char filename[100] = FS_ROOT;	
	int iRet;
	strcat(filename, "/testdir2");	
	/*prepare*/
	rmdir(filename);
	iRet = mkdir(filename, 0);
	if(-1 == iRet)
	{
       TEST_FAILRINT();
	   c_Results.isPass=PTS_FAIL;

		return PTS_FAIL;
	}
	/*test*/
	iRet = rmdir(filename);
	if(0 == iRet)
	{
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
