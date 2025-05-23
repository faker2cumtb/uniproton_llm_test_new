
/* 
【测试目的】:
调用stat获得指定路径下文件的信息，如果成功获得<path>指定路径下文件的信息，应将其信息保存至<sbuf>，并返回0。

【测试类型】:
功能性测试

【先决条件】:
1.文件系统正常运行

【测试步骤】:
1.声明struct stat类型的便利tmp和指针变量sbuf，声明长度为37的wrbuf数组，初始化为字符串"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"；2.调用tstdef_CreateAndWriteFile创建文件/vnfs/test.txt，并将wrbuf字符写入文件；3.调用stat获取/vnfs/test.txt的文件信息，并将返回值赋值给iRet；4.判断iRet的值是否为0，若iRet为0并且文件信息的长度为36，则返回测试成功标志，否则测试失败，返回测试失败标志。

【预期结果】:
返回PASS

【评价准则】:
与预期的测试结果一致

 */


/****************************** 实现部分 *********************************/
#include <stdio.h>
#include <test.h>
#include "fcntl.h"
#include <sys/stat.h>
int OS_FS_Stat_01()
{
	struct stat tmp;
	struct stat *sbuf;
	char wrbuf[]="aaaaaa";
	int fd;
	char filename[100]= FS_ROOT;
	int iRet;
	int failed = 0;
	strcat(filename, "/test.txt");
	sbuf = &tmp;
	/*prepare*/
	iRet=tstdef_CreateAndWriteFile(filename,wrbuf);
	if(iRet==AFILE_FAILED)
	{
	   TEST_FAILRINT();
	   c_Results.isPass=PTS_FAIL;

		return PTS_FAIL;
	}

	/*test*/
	iRet=stat(filename,sbuf);
	printf("%d\n",sbuf->st_size);
	remove(filename);
	if((iRet==0) && sbuf->st_size == 6)
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
