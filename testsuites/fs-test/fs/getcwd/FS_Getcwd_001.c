
/* 
【测试目的】:
当设置路径保存缓冲区并设置长度为256时，测试getcwd获取当前工作目录是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
设置路径保存缓冲区并设置长度为256；获取当前工作路径。

【预期结果】:
获取当前默认路径成功，返回当前工作目录的绝对路径。

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
int	OS_FS_Getcwd_001()
{
    char *ret;
    char buffer[256];

	chdir(FS_ROOT);
    /*获取当前默认路径*/
    ret = getcwd(buffer, 256);
    if (ret == NULL )
    {
        TEST_ERRPRINT("getcwd EINVAL");
        return PTS_FAIL;
    }
    TEST_OKPRINT();
    return PTS_PASS;
}
