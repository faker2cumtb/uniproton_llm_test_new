/* 
【测试目的】:
当设置pathname等于NULL时，测试chdir改变目录是否成功,错误号是否为EINVAL。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用接口chdir改变当前工作目录，设置pathname等于NULL；判断返回值。

【预期结果】:
返回值等于-1。 错误号为EINVAL。

【评价准则】:
与预期的测试结果一致

*/

#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>

int OS_FS_Chdir_EINVAL()
{
    int ret;
    char *ptr=NULL;

    ret = chdir(ptr);
    if ( ret != -1 || errno != EINVAL)//参数NULL，设置errno为EINVAL
    {
    	TSTDEF_FAILPRINT(errno);
        return PTS_FAIL;
    }

    TEST_OKPRINT();
    return PTS_PASS;
}
