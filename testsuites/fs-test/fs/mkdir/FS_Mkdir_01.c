
/* 
【测试目的】:
调用mkdir用于在文件系统中创建指定名称的目录，如果在文件系统中成功创建<path>指定的目录，应返回0。

【测试类型】:
功能性测试

【先决条件】:
1.文件系统正常运行

【测试步骤】:
1.声明char类型的数组filename，初始化为 "/vnfs/test"；2.以filename和0为参数调用mkdir新建文件目录，并将返回值赋给iRet；3.判断返回值iRet为0，则测试成功，返回测试成功标志，否则测试失败，返回测试失败标志。

【预期结果】:
返回PASS

 */


/****************************** 实现部分 *********************************/
#include <stdio.h>
#include <test.h>
#include "fcntl.h"
int OS_FS_Mkdir_01()
{
	char filename[100] = FS_ROOT;	
	int iRet1;
	strcat(filename, "/testMKDIR1");
	/*test*/	
	rmdir(filename);
	iRet1= mkdir(filename, 0);
	if(0 == iRet1)
	{
        TEST_OKPRINT();
	    c_Results.isPass=PTS_PASS;
		rmdir(filename);
		return PTS_PASS;
	}
	
	else
	{
        TEST_FAILRINT();
	    c_Results.isPass=PTS_FAIL;
		rmdir(filename);
		return PTS_FAIL;		
	}
}
