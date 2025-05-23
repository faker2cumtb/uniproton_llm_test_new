
/* 

【测试目的】:
当存放路径的缓冲区为空时，测试getcwd获取当前工作目录是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
设置路径保存缓冲区为NULL；获取当前默认路径，是否返回NULL。

【预期结果】:
获取当前默认路径失败，返回NULL；

【评价准则】:
与预期的测试结果一致
 */


/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>

/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Getcwd_EINVAL2()
{
    char *ret;
    /*获取当前默认路径*/
    ret = getcwd(NULL, 16);
    if ( (ret != NULL )&&( errno != EINVAL))//posix标准：If buf is a null pointer, the behavior of getcwd() is unspecified
    {
    	TSTDEF_FAILPRINT(errno);
    	printf("%s \n",ret);
        return PTS_FAIL;
    }
    TEST_OKPRINT();
    return PTS_PASS;
}
