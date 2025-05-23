/* 
【测试目的】:
当输入参数有效时，测试chdir接口,将 pathname 指向的目录设置为当前的工作路径 是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用接口getcwd获取当前工作目录；调用接口chdir改变当前工作目录；再调用接口getcwd获取当前工作目录；调用接口strcmp比较前后工作目录是否变化。

【预期结果】:
返回值不等于0。
 */
#include <errno.h>
#include "test.h"

int OS_FS_Chdir_001()
{
    int ret;
    int failed = 0;
    int	 iRet;
    char *toPos;
    char path[1024];
    
    ret = chdir(FS_TMP);
    if (ret == -1)
    {
        TEST_ERRPRINT("test errno");
        failed = 1;
    }
    toPos = getcwd(path, 1024);
    printf("-----%s----\n",  toPos);
    if (toPos == NULL)
    {
        TEST_ERRPRINT("getcwd");
        failed = 1;
    }

    iRet = strcmp(toPos, FS_TMP);
    if (iRet != 0)
    {
        TEST_ERRPRINT("strcmp");
        failed = 1;
    }

    ret = chdir(FS_ROOT);
    if (ret == -1)
    {
        TEST_ERRPRINT("test errno");
        failed = 1;
    }

    toPos = getcwd(path, 1024);
    printf("-----%s----\n",  toPos);
    if (toPos == NULL)
    {
        TEST_ERRPRINT("getcwd");
        failed = 1;
    }
    iRet = strcmp(toPos, FS_ROOT);
    if (iRet != 0)
    {
        TEST_ERRPRINT("strcmp");
        failed = 1;
    }

    if (!failed)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("TEST FAIL\n");
        return PTS_FAIL;
    }
}
