
/* 

【测试目的】:
调用close关闭指定文件，如果关闭失败则返回-1.

【测试类型】:
功能性测试

【先决条件】:
1.文件系统正常运行

【测试步骤】:
1.声明int型的变量fd，初始化为-22；2.调用close关闭文件，参数为fd，将返回值赋值给ret；3.判断返回值ret为-1，则测试成功，返回测试成功标志，否则测试失败，返回测试失败标志。

【预期结果】:
返回PASS

【评价准则】:
与预期的测试结果一致

*/


/****************************** 实现部分 *********************************/
#include <stdio.h>
#include <test.h>
#include "fcntl.h"

int OS_FS_Close_02()
{
	int fd = -22;
	int ret;
	/*关闭文件*/
	ret = close(fd);
	/*判断是否关闭失败，如果失败则测试成功，否则测试失败*/		
	if(ret == -1)
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
